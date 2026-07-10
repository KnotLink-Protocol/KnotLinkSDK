// KnotLink SDK - Rust
// Copyright (c) 2024-2026 KnotLink Contributors
// SPDX-License-Identifier: MIT

use anyhow::Result;
use tokio::sync::mpsc;
use log::error;
use super::tcp_client::TcpClient;

const SUBSCRIBER_ADDR: &str = "127.0.0.1:6372";

pub struct SignalSubscriber {
    pub rx: mpsc::UnboundedReceiver<String>,
    _task_handle: tokio::task::JoinHandle<()>,
}

impl SignalSubscriber {
    pub async fn new(app_id: String, signal_id: String) -> Result<Self> {
        let (client, tx) = TcpClient::connect(SUBSCRIBER_ADDR, 180).await?;
        let reg = format!("{}-{}", app_id, signal_id);
        tx.send(reg.into_bytes())?;

        let (msg_tx, msg_rx) = mpsc::unbounded_channel();
        let handle = tokio::spawn(async move {
            let on_data = move |data: Vec<u8>| {
                if let Ok(s) = String::from_utf8(data) {
                    let _ = msg_tx.send(s);
                }
            };
            if let Err(e) = client.run(on_data).await {
                error!("SignalSubscriber TCP error: {}", e);
            }
        });

        Ok(SignalSubscriber {
            rx: msg_rx,
            _task_handle: handle,
        })
    }
}
