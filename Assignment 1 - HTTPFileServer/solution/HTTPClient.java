import java.io.File;
import java.io.IOException;
import java.net.Socket;
import java.util.Scanner;

public class HTTPClient {
    public static void main(String[] args) {
        Scanner in = new Scanner(System.in);
        while (true) {
            String filePath = in.nextLine();
            new Thread(()->{
                try {
                    Socket socket = new Socket("localhost", HTTPServer.PORT);
                    System.out.println("Connection established");
                    System.out.println("Remote port: "+socket.getPort()+" Local port: "+socket.getLocalPort());
                    NetworkUtil networkUtil = new NetworkUtil(socket);
                    File file = new File(filePath);
                    if (!file.isFile()) {
                        System.out.println("invalid file path");
                        networkUtil.write("UPLOAD invalid/file\n");
                    } else {
                        networkUtil.write("UPLOAD " + file.getName()+"\n");
                        networkUtil.sendFile(filePath);
                    }
                    networkUtil.closeConnection();
                } catch (IOException e) {
                    e.printStackTrace();
                }
            }).start();
        }
    }
}
