import java.io.*;
import javax.net.ssl.SSLSocket;
import javax.net.ssl.SSLSocketFactory;

public class TcpClient0 {
    public static void main(String[] args) {
        String host = "wokwi.com";

        try {
            SSLSocketFactory factory =
                (SSLSocketFactory) SSLSocketFactory.getDefault();

            try (SSLSocket socket =
                    (SSLSocket) factory.createSocket(host, 443)) {

                PrintWriter out =
                    new PrintWriter(socket.getOutputStream(), true);

                BufferedReader in =
                    new BufferedReader(
                        new InputStreamReader(socket.getInputStream()));

                out.println("GET / HTTP/1.1");
                out.println("Host: " + host);
                out.println("Connection: close");
                out.println();

                String line;
                while ((line = in.readLine()) != null) {
                    System.out.println(line);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}