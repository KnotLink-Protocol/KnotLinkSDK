// KnotLink SDK - Rust
// Copyright (c) 2024-2026 KnotLink Contributors
// SPDX-License-Identifier: MIT

use anyhow::Result;
use tokio::sync::mpsc;
use log::error;
use super::tcp_client::TcpClient;

const SENDER_ADDR: &str = "127.0.0.1:6370";

pub struct SignalSender {
    tx: mpsc::UnboundedSender<Vec<u8>>,
    _task_handle: tokio::task::JoinHandle<()>,
}

impl SignalSender {
    pub async fn new() -> Result<Self> {
        let (client, tx) = TcpClient::connect(SENDER_ADDR, 180).await?;
        let handle = tokio::spawn(async move {
            let _ = client.run(|_| {}).await;
        });
        Ok(SignalSender {
            tx,
            _task_handle: handle,
        })
    }

    pub fn emit(&self, app_id: &str, signal_id: &str, data: String) -> Result<()> {
        let key = format!("{}-{}&*&", app_id, signal_id);
        let mut full = key.into_bytes();
        full.extend(data.as_bytes());
        self.tx.send(full)?;
        Ok(())
    }
}
