use anyhow::Result;
use tokio::sync::mpsc;
use super::tcp_client::TcpClient;

pub struct SignalSubscriber {
    pub rx: mpsc::UnboundedReceiver<String>,
    _task_handle: tokio::task::JoinHandle<()>,
}

impl SignalSubscriber {
    pub async fn new(app_id: String, signal_id: String, server_addr: &str) -> Result<Self> {
        let (client, tx) = TcpClient::connect(server_addr, 180).await?;
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
                eprintln!("SignalSubscriber TCP 错误: {}", e);
            }
        });

        Ok(SignalSubscriber {
            rx: msg_rx,
            _task_handle: handle,
        })
    }
}