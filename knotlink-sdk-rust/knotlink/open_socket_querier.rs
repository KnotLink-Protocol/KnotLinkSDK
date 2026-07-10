// KnotLink SDK - Rust
// Copyright (c) 2024-2026 KnotLink Contributors
// SPDX-License-Identifier: MIT

use anyhow::Result;
use std::sync::Arc;
use std::sync::Mutex;
use tokio::sync::{mpsc, oneshot};
use tokio::time::{self, Duration};
use log::{debug, error};
use super::tcp_client::TcpClient;

const QUERIER_ADDR: &str = "127.0.0.1:6376";
pub struct OpenSocketQuerier {
    tx: mpsc::UnboundedSender<Vec<u8>>,
    pending_response: Arc<Mutex<Option<oneshot::Sender<Vec<u8>>>>>,
    _task_handle: tokio::task::JoinHandle<()>,
}

impl OpenSocketQuerier {
    pub async fn new(app_id: String, open_socket_id: String) -> Result<Self> {
        let (client, tx) = TcpClient::connect(QUERIER_ADDR, 180).await?;
        let pending = Arc::new(Mutex::new(None::<oneshot::Sender<Vec<u8>>>));
        let pending_clone = pending.clone();

        let handle = tokio::spawn(async move {
            let on_data = move |data: Vec<u8>| {
                if let Ok(mut guard) = pending_clone.lock() {
                    if let Some(sender) = guard.take() {
                        let _ = sender.send(data);
                    }
                }
            };
            if let Err(e) = client.run(on_data).await {
                error!("OpenSocketQuerier TCP error: {}", e);
            }
        });

        // 保存 app_id/open_socket_id，后续 query 时拼 key
        let q = OpenSocketQuerier {
            tx,
            pending_response: pending,
            _task_handle: handle,
        };

        // 立即注册
        tx.send(format!("{}-{}", app_id, open_socket_id).into_bytes())?;
        Ok(q)
    }

    /// 同步查询，可选超时
    pub async fn query_l(&self, data: String, app_id: &str, open_socket_id: &str,
                         timeout: Option<Duration>) -> Result<String> {
        // timeout=None 时不超时
        let key = format!("{}-{}&*&", app_id, open_socket_id);
        let mut full = key.into_bytes();
        full.extend(data.as_bytes());
        self.tx.send(full)?;

        let (resp_tx, resp_rx) = oneshot::channel();
        {
            let mut guard = self.pending_response.lock().unwrap();
            *guard = Some(resp_tx);
        }

        let result = match timeout {
            Some(d) => time::timeout(d, resp_rx).await.map_err(|_|
                anyhow::anyhow!("query_l timed out after {:?}", d))?,
            None => resp_rx.await,
        };

        result.map_err(|_| anyhow::anyhow!("Response channel closed"))
            .and_then(|r| Ok(String::from_utf8(r)?))
    }

    /// 异步查询，不等待响应
    pub async fn query(&self, data: String, app_id: &str, open_socket_id: &str) -> Result<()> {
        let key = format!("{}-{}&*&", app_id, open_socket_id);
        let mut full = key.into_bytes();
        full.extend(data.as_bytes());
        self.tx.send(full)?;
        Ok(())
    }
}
