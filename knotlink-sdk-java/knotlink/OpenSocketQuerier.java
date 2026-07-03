/*
 * KnotLink SDK - Java
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

import java.io.EOFException;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.Socket;
import java.net.SocketTimeoutException;
import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.concurrent.CompletableFuture;
import java.util.logging.Level;
import java.util.logging.Logger;

public class OpenSocketQuerier {
    private static final String SERVER_IP = "127.0.0.1";
    private static final int QUERIER_PORT = 6376;
    private static final int SOCKET_TIMEOUT_MS = 5000;
    private static final int MAX_MSG_SIZE = 16 * 1024 * 1024; // 16MB
    private static final Logger LOGGER = Logger.getLogger(OpenSocketQuerier.class.getName());

    public static CompletableFuture<String> query(String appID, String openSocketID, String question) {
        return CompletableFuture.supplyAsync(() -> {
            try (Socket socket = new Socket(SERVER_IP, QUERIER_PORT)) {
                socket.setSoTimeout(SOCKET_TIMEOUT_MS);

                String packet = String.format("%s-%s&*&%s", appID, openSocketID, question);
                LOGGER.info(() -> "Sending query to KnotLink: " + question);
                writeWithLengthPrefix(socket.getOutputStream(), packet.getBytes(StandardCharsets.UTF_8));

                byte[] response = readLengthPrefixedResponse(socket.getInputStream());
                String responseText = new String(response, StandardCharsets.UTF_8);
                System.out.println("Received query response: " + responseText);
                return responseText;
            } catch (SocketTimeoutException e) {
                LOGGER.log(Level.WARNING, "Timed out querying KnotLink server", e);
                return "ERROR: Query timed out.";
            } catch (Exception e) {
                LOGGER.log(Level.WARNING, "Failed to query KnotLink server", e);
                return "ERROR: Cannot connect to KnotLink. Ensure the service is running.";
            }
        });
    }

    private static void writeWithLengthPrefix(OutputStream out, byte[] data) throws IOException {
        if (data.length > MAX_MSG_SIZE) {
            throw new IOException("Message too large: " + data.length);
        }

        ByteBuffer lengthBuf = ByteBuffer.allocate(4);
        lengthBuf.putInt(data.length);
        out.write(lengthBuf.array());
        out.write(data);
        out.flush();
    }

    private static byte[] readLengthPrefixedResponse(InputStream in) throws IOException {
        byte[] lengthBytes = readExactly(in, 4);
        int length = ByteBuffer.wrap(lengthBytes).getInt();
        if (length <= 0 || length > MAX_MSG_SIZE) {
            throw new IOException("Invalid message length: " + length);
        }
        return readExactly(in, length);
    }

    private static byte[] readExactly(InputStream in, int length) throws IOException {
        byte[] data = new byte[length];
        int offset = 0;
        while (offset < length) {
            int read = in.read(data, offset, length - offset);
            if (read == -1) {
                throw new EOFException("Stream closed before reading " + length + " bytes");
            }
            offset += read;
        }
        return data;
    }
}
