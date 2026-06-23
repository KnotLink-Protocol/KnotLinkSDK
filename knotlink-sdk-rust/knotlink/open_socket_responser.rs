// KnotLink SDK - Rust
// Copyright (c) 2024-2026 KnotLink Contributors
// SPDX-License-Identifier: MIT

use anyhow::Result;
use tokio::sync::mpsc;
use log::info;
use super::tcp_client::TcpClient;

pub struct OpenSocketResponser {
    tx: mpsc::UnboundedSender<Vec<u8>>,
    pub rx: mpsc::UnboundedReceiver<(String, String)>, // (question_id, data)
    _task_handle: tokio::task::JoinHandle<()>,
}

impl OpenSocketResponser {
    pub async fn new(app_id: String, open_socket_id: String, server_addr: &str) -> Result<Self> {
        let (client, tx) = TcpClient::connect(server_addr, 180).await?;
        let reg = format!("{}-{}", app_id, open_socket_id);
        tx.send(reg.into_bytes())?;

        let (msg_tx, msg_rx) = mpsc::unbounded_channel();
        let tx_clone = tx.clone();

        let handle = tokio::spawn(async move {
            let on_data = move |data: Vec<u8>| {
                if let Ok(s) = String::from_utf8(data) {
                    if let Some((key, t_data)) = s.split_once("&*&") {
                        let _ = msg_tx.send((key.to_string(), t_data.to_string()));
                        info!("收到请求: key={}, data={}", key, t_data);
                    } else {
                        eprintln!("无效数据格式: {}", s);
                    }
                }
            };
            if let Err(e) = client.run(on_data).await {
                eprintln!("OpenSocketResponser TCP 错误: {}", e);
            }
        });

        Ok(OpenSocketResponser {
            tx: tx_clone,
            rx: msg_rx,
            _task_handle: handle,
        })
    }

    /// 发送响应给指定的 question_id
    pub async fn send_back(&self, question_id: &str, data: &str) -> Result<()> {
        let resp = format!("{}&*&{}", question_id, data);
        self.tx.send(resp.into_bytes())?;
        Ok(())
    }
}