/*
 * KnotLink SDK - Java regression checks
 * Copyright (c) 2024-2026 KnotLink Contributors
 * SPDX-License-Identifier: MIT
 */

import java.io.IOException;
import java.nio.charset.StandardCharsets;
import java.nio.file.Files;
import java.nio.file.Path;

public class JavaSdkRegressionTest {
    private static final Path ROOT = Path.of(System.getProperty("knotlink.repoRoot", ".")).toAbsolutePath().normalize();

    public static void main(String[] args) throws Exception {
        String signalSender = read("knotlink-sdk-java/knotlink/SignalSender.java");
        String responser = read("knotlink-sdk-java/knotlink/OpenSocketResponser.java");
        String querier = read("knotlink-sdk-java/knotlink/OpenSocketQuerier.java");
        String tcpClient = read("knotlink-sdk-java/knotlink/TcpClient.java");

        assertContains("SignalSender must connect to sender port 6370",
                signalSender, "connectToServer(\"127.0.0.1\", 6370)");
        assertContains("OpenSocketResponser must connect to responser port 6378",
                responser, "connectToServer(\"127.0.0.1\", 6378)");
        assertContains("OpenSocketQuerier must use length-prefixed writes",
                querier, "writeWithLengthPrefix");
        assertContains("OpenSocketQuerier must read length-prefixed responses",
                querier, "readLengthPrefixedResponse");
        assertContains("TcpClient must reject negative lengths",
                tcpClient, "length <= 0");
        assertContains("OpenSocketResponser split must keep payload delimiters",
                responser, "split(delimiter, 2)");
        assertContains("OpenSocketResponser byte[] sendBack must preserve bytes",
                responser, "KLresponser.sendData(packet)");
        assertContains("TcpClient string decoding must use explicit UTF-8",
                tcpClient, "StandardCharsets.UTF_8");

        System.out.println("Java SDK regression checks passed.");
    }

    private static String read(String relativePath) throws IOException {
        return Files.readString(ROOT.resolve(relativePath), StandardCharsets.UTF_8);
    }

    private static void assertContains(String description, String content, String expected) {
        if (!content.contains(expected)) {
            throw new AssertionError(description + ": missing `" + expected + "`");
        }
    }
}
