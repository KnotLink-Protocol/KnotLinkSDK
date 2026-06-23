// KnotLink SDK - Rust
// Copyright (c) 2024-2026 KnotLink Contributors
// SPDX-License-Identifier: MIT

use anyhow::Result;
use tokio::sync::mpsc;
use super::tcp_client::TcpClient;

pub struct SignalSender {
    app_id: String,
    signal_id: String,
    tx: mpsc::UnboundedSender<Vec<u8>>,
    _task_handle: tokio::task::JoinHandle<()>,
}

impl SignalSender {
    pub async fn new(app_id: String, signal_id: String, server_addr: &str) -> Result<Self> {
        let (client, tx) = TcpClient::connect(server_addr, 180).await?;
        let handle = tokio::spawn(async move {
            // 信号发送器只发送，不需要处理接收数据
            let _ = client.run(|_| {}).await;
        });
        Ok(SignalSender {
            app_id,
            signal_id,
            tx,
            _task_handle: handle,
        })
    }

    pub async fn emit(&self, data: String) -> Result<()> {
        let key = format!("{}-{}&*&", self.app_id, self.signal_id);
        let mut full = key.into_bytes();
        full.extend(data.as_bytes());
        self.tx.send(full)?;
        Ok(())
    }
}