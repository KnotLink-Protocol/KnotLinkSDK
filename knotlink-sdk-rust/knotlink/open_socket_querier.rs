// KnotLink SDK - Rust
// Copyright (c) 2024-2026 KnotLink Contributors
// SPDX-License-Identifier: MIT

use anyhow::Result;
use std::sync::Arc;
use std::sync::Mutex;          // 改用标准库的 Mutex
use tokio::sync::{mpsc, oneshot};
use log::debug;
use super::tcp_client::TcpClient;

pub struct OpenSocketQuerier {
    app_id: String,
    open_socket_id: String,
    tx: mpsc::UnboundedSender<Vec<u8>>,
    pending_response: Arc<Mutex<Option<oneshot::Sender<Vec<u8>>>>>,
    _task_handle: tokio::task::JoinHandle<()>,
}

impl OpenSocketQuerier {
    pub async fn new(app_id: String, open_socket_id: String, server_addr: &str) -> Result<Self> {
        let (client, tx) = TcpClient::connect(server_addr, 180).await?;
        let pending = Arc::new(Mutex::new(None::<oneshot::Sender<Vec<u8>>>));
        let pending_clone = pending.clone();

        let handle = tokio::spawn(async move {
            let on_data = move |data: Vec<u8>| {
                // 使用标准库的锁（不会 panic）
                if let Ok(mut guard) = pending_clone.lock() {
                    if let Some(sender) = guard.take() {
                        let _ = sender.send(data);
                    }
                }
            };
            if let Err(e) = client.run(on_data).await {
                eprintln!("OpenSocketQuerier TCP 错误: {}", e);
            }
        });

        Ok(OpenSocketQuerier {
            app_id,
            open_socket_id,
            tx,
            pending_response: pending,
            _task_handle: handle,
        })
    }

    pub async fn query_l(&self, data: String) -> Result<String> {
        let key = format!("{}-{}&*&", self.app_id, self.open_socket_id);
        let mut full = key.into_bytes();
        full.extend(data.as_bytes());
        self.tx.send(full)?;

        let (resp_tx, resp_rx) = oneshot::channel();
        {
            let mut guard = self.pending_response.lock().unwrap();
            *guard = Some(resp_tx);
        }
        let resp = resp_rx.await.map_err(|_| anyhow::anyhow!("响应通道关闭"))?;
        Ok(String::from_utf8(resp)?)
    }

    pub async fn query(&self, data: String) -> Result<()> {
        let key = format!("{}-{}&*&", self.app_id, self.open_socket_id);
        let mut full = key.into_bytes();
        full.extend(data.as_bytes());
        self.tx.send(full)?;
        Ok(())
    }
}