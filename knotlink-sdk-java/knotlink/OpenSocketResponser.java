/*
 * KnotLink SDK - Java
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

import java.nio.charset.StandardCharsets;

public class OpenSocketResponser implements AutoCloseable {
    private TcpClient KLresponser;
    private String appID;
    private String openSocketID;

    public interface DataListener {
        void onDataReceived(String data, String key);
    }

    private DataListener dataListener;

    public OpenSocketResponser(String appID, String openSocketID) {
        this.appID = appID;
        this.openSocketID = openSocketID;
        init();
    }

    public void setDataListener(DataListener listener) {
        this.dataListener = listener;
    }

    private void init() {
        KLresponser = new TcpClient();
        KLresponser.connectToServer("127.0.0.1", 6378);

        KLresponser.setDataReceivedListener(this::dataRecv);

        String s_key = appID + "-" + openSocketID;
        KLresponser.sendData(s_key); // 发送初始化数据
    }

    private void dataRecv(String s_data) {
        String delimiter = "&\\*&"; // 分隔符
        String[] parts = s_data.split(delimiter, 2);

        if (parts.length != 2) {
            System.err.println("Invalid data format. Expected two parts separated by &*&");
            return;
        }

        String key = parts[0]; // 前一部分作为 key
        String t_data = parts[1]; // 后一部分作为 t_data

        // 调用外部回调
        if (dataListener != null) {
            dataListener.onDataReceived(t_data, key);
        }
    }

    public void sendBack(String data, String questionID) {
        sendBack(data.getBytes(StandardCharsets.UTF_8), questionID);
    }

    public void sendBack(byte[] data, String questionID) {
        byte[] prefix = (questionID + "&*&").getBytes(StandardCharsets.UTF_8);
        byte[] packet = new byte[prefix.length + data.length];
        System.arraycopy(prefix, 0, packet, 0, prefix.length);
        System.arraycopy(data, 0, packet, prefix.length, data.length);
        KLresponser.sendData(packet);
    }

    @Override
    public void close() {
        KLresponser.close();
    }
}
