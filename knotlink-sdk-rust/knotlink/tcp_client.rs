// KnotLink SDK - Rust
// Copyright (c) 2024-2026 KnotLink Contributors
// SPDX-License-Identifier: MIT

use anyhow::Result;
use bytes::{BytesMut, Buf};
use tokio::io::{AsyncReadExt, AsyncWriteExt};
use tokio::net::TcpStream;
use tokio::sync::mpsc;
use tokio::time::{self, Duration};
use log::{debug, error, info, warn};

const MAX_MSG_SIZE: u32 = 16 * 1024 * 1024; // 16MB，可根据需要调整
const MAGIC: &[u8; 4] = &[0x4B, 0x4B, 0x00, 0x02]; // KK + 版本号 2.0
const MAGIC_LEN: usize = 4;
const HEADER_LEN: usize = MAGIC_LEN + 4; // 8: 魔数 + 长度字段

pub struct TcpClient {
    stream: TcpStream,
    rx: mpsc::UnboundedReceiver<Vec<u8>>,
    heartbeat_interval: Duration,
}

impl TcpClient {
    pub async fn connect(addr: &str, heartbeat_secs: u64) -> Result<(Self, mpsc::UnboundedSender<Vec<u8>>)> {
        let stream = TcpStream::connect(addr).await?;
        let (tx, rx) = mpsc::unbounded_channel();
        let client = TcpClient {
            stream,
            rx,
            heartbeat_interval: Duration::from_secs(heartbeat_secs),
        };
        Ok((client, tx))
    }

    /// 打包消息：长度（4字节大端） + 数据
    fn pack_message(data: &[u8]) -> Vec<u8> {
        let len = data.len() as u32;
        let mut buf = Vec::with_capacity(HEADER_LEN + len as usize);
        buf.extend_from_slice(MAGIC.as_slice());
        buf.extend_from_slice(&len.to_be_bytes());
        buf.extend_from_slice(data);
        buf
    }

    pub async fn run(self, mut on_data: impl FnMut(Vec<u8>) + Send + 'static) -> Result<()> {
        let TcpClient { mut stream, mut rx, heartbeat_interval } = self;
        let mut heartbeat_timer = time::interval(heartbeat_interval);
        let mut read_buf = BytesMut::with_capacity(4096);

        loop {
            tokio::select! {
                // ---------- 发送业务数据（自动加长度前缀） ----------
                Some(data) = rx.recv() => {
                    let packet = Self::pack_message(&data);
                    if let Err(e) = stream.write_all(&packet).await {
                        error!("发送数据失败: {}", e);
                        return Err(e.into());
                    }
                    debug!("发送业务数据 {} 字节 (含前缀)", packet.len());
                }

                // ---------- 发送心跳（同样加长度前缀） ----------
                _ = heartbeat_timer.tick() => {
                    let heartbeat_packet = Self::pack_message(b"heartbeat");
                    if let Err(e) = stream.write_all(&heartbeat_packet).await {
                        error!("发送心跳失败: {}", e);
                        return Err(e.into());
                    }
                    debug!("发送心跳包");
                }

                // ---------- 接收数据，解析完整消息 ----------
                result = stream.read_buf(&mut read_buf) => {
                    match result {
                        Ok(0) => {
                            info!("服务器关闭连接");
                            return Ok(());
                        }
                        Ok(n) => {
                            debug!("收到 {} 字节原始数据", n);
                            // 循环解析缓冲区中的所有完整消息
                            loop {
                                if read_buf.len() < HEADER_LEN {
                                    break; // 协议头未完整
                                }
                                // 校验魔数
                                if &read_buf[..MAGIC_LEN] != MAGIC.as_slice() {
                                    warn!("Magic mismatch, disconnecting");
                                    return Err(anyhow::anyhow!("Magic mismatch"));
                                }
                                // 读取长度（大端，在魔数之后）
                                let len = u32::from_be_bytes(read_buf[MAGIC_LEN..HEADER_LEN].try_into().unwrap());
                                if len == 0 || len > MAX_MSG_SIZE {
                                    warn!("无效消息长度: {}, 断开连接", len);
                                    return Err(anyhow::anyhow!("Invalid message length: {}", len));
                                }
                                let total_len = HEADER_LEN + len as usize;
                                if read_buf.len() < total_len {
                                    break; // 消息体未完整
                                }
                                // 提取消息体（跳过魔数 + 长度）
                                let msg = read_buf[HEADER_LEN..total_len].to_vec();
                                read_buf.advance(total_len);
                                // 处理消息
                                if msg == b"heartbeat_response" {
                                    debug!("收到心跳响应，忽略");
                                } else {
                                    debug!("收到应用消息 {} 字节", msg.len());
                                    on_data(msg);
                                }
                                // 继续循环，可能还有更多消息
                            }
                        }
                        Err(e) => {
                            error!("读取数据失败: {}", e);
                            return Err(e.into());
                        }
                    }
                }
            }
        }
    }
}