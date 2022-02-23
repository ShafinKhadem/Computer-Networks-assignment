import java.io.*;
import java.net.ServerSocket;
import java.net.Socket;
import java.util.Date;

public class HTTPServer {
    static final int PORT = 6789;
    static int cntClient = 0;
    
    public static void main(String[] args) {
    
        try (ServerSocket serverSocket = new ServerSocket(PORT)) {
            System.out.println("Server started.\nListening for connections on port : " + PORT + " ...\n");
            
            while(true) {
                System.out.println ("Waiting for new client");
                final Socket socket = serverSocket.accept();
                System.out.println("Remote port: "+socket.getPort()+" Local port: "+socket.getLocalPort());
                final NetworkUtil networkUtil = new NetworkUtil(socket);
                cntClient++;
                final int clientId = cntClient;
    
                new Thread(()->{
                    String input = networkUtil.readLine();
                    System.out.println(clientId+" "+socket.getPort()+" "+input);

                    String content, status, contentType;
                    if(input == null || input.length()==0) return;

                    if(input.startsWith("GET")) {
                        PrintWriter filePrintWriter = null;
                        try {
                            filePrintWriter = new PrintWriter(cntClient+"_"+socket.getPort()+".log");
                        } catch (FileNotFoundException e) {
                            e.printStackTrace();
                        }
                        filePrintWriter.println("Request:");
                        filePrintWriter.println(input);
                        input = input.substring(5, input.length()-9).replace("%20", " ");
                        String path = input.equals("") ? "root" : input;
                        System.out.println(path);
                        File f = new File(path);
                        if (f.isFile ()) {
                            networkUtil.sendFileHTTP(path, filePrintWriter);
                            input = networkUtil.readLine();
                            while (!input.isEmpty()) {
                                System.out.println(clientId+" "+socket.getPort()+" "+input);
                                input = networkUtil.readLine();
                            }
                            networkUtil.closeConnection();
                            return;
                        }

                        content = "<html>\n" +
                                  "\t<head>\n" +
                                  "\t\t<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n" +
                                  "\t\t<link rel=\"icon\" href=\"data:,\">\n"+
                                  "\t\t<base href=\"http://localhost:"+PORT+"/\">\n" +
                                  "\t</head>\n" +
                                  "\t<body>";
                        contentType = "text/html";

                        String[] names = f.list();
                        if (names==null) {
                            content += "\n404: Page not found";
                            System.out.println("404: Page not found");
                            status = "404 Not Found";
                        } else {
                            for (String name : names) {
                                String childName = path+"/"+name;
                                if (new File(childName).isDirectory())
                                    content += "\n\t\t<a style=\"font-weight:bold\" href=\""+childName+"\">"+name+"</a><br>";
                                else
                                    content += "\n\t\t<a href=\""+childName+"\">"+name+"</a><br>";
                            }
                            status = "200 OK";
                        }

                        content += "\n\t</body>\n</html>\n";
//                            System.out.println(content);
//                            System.out.println(contentType);
//                            System.out.println(status);
                        networkUtil.write("HTTP/1.1 "+status+"\r\n");
                        networkUtil.write("Server: Java HTTP Server: 1.0\r\n");
                        networkUtil.write("Date: " + new Date() + "\r\n");
                        networkUtil.write("Content-Type: "+contentType+"\r\n");
                        networkUtil.write("Content-Length: " + content.length() + "\r\n");
                        networkUtil.write("\r\n");
                        networkUtil.write(content);
                        
                        filePrintWriter.println("Response:");
                        filePrintWriter.println("HTTP/1.1 "+status+"\r\n");
                        filePrintWriter.println("Server: Java HTTP Server: 1.0\r\n");
                        filePrintWriter.println("Date: " + new Date() + "\r\n");
                        filePrintWriter.println("Content-Type: "+contentType+"\r\n");
                        filePrintWriter.println("Content-Length: " + content.length() + "\r\n");
                        filePrintWriter.println("\r\n");
                        filePrintWriter.println(content);
                        filePrintWriter.close();
                    } else if (input.startsWith("UPLOAD")) {
                        String fileName = input.substring(7);
                        if (fileName.equals("invalid/file")) {
                            System.out.println("Invalid file");
                        } else {
                            networkUtil.receiveFile("root/"+fileName);
                        }
                    }
                    input = networkUtil.readLine();
                    while (!input.isEmpty()) {
                        System.out.println(clientId+" "+socket.getPort()+" "+input);
                        input = networkUtil.readLine();
                    }
                    networkUtil.closeConnection();
                }).start();
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        
    }
    
}
