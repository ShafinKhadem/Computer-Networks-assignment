import java.io.*;
import java.net.Socket;
import java.nio.file.Files;
import java.util.Date;

public class NetworkUtil {
    private Socket socket;
    private InputStream is;
    private OutputStream os;
    private PrintWriter pr;    // printWriter and outputStream can be used together using flush, but reader and
    // inputStream can't be used together, as inputStreamReader can read extra bytes, even
    // more so when wrapped by BufferedReader
    
    public NetworkUtil(Socket s) {//To make object from socket with all necessary methods
        // stream can be created from a socket's stream only once.
        socket = s;
        try {
            os = s.getOutputStream();
            pr = new PrintWriter(os);
            is = s.getInputStream();
        } catch (Exception e) {
            System.out.println("Exception In constructing NetworkUtil : ");
            e.printStackTrace(System.out);
        }
    }
    
    public String readLine() {
        try {
            StringBuilder stringBuilder = new StringBuilder();
            
            for (int c; (c = is.read())!='\n' && c!=-1; ) {
                stringBuilder.append((char) c);
            }
            if (stringBuilder.length()>0 && stringBuilder.charAt(stringBuilder.length()-1)=='\r') {
                stringBuilder.setLength(stringBuilder.length()-1);
            }
            return stringBuilder.toString();
        } catch (IOException e) {
            e.printStackTrace();
            return null;
        }
    }
    
    public int read(byte[] b) {
        try {
            return is.read(b);
        } catch (IOException e) {
            e.printStackTrace();
            return 0;
        }
    }
    
    public int read(byte[] b, int off, int len) {
        try {
            return is.read(b, off, len);
        } catch (IOException e) {
            e.printStackTrace();
            return 0;
        }
    }
    
    public void write(String s) {
        pr.write(s);
        pr.flush();
    }
    
    public void write(byte[] b) {
        try {
            os.write(b);
            os.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    public void write(byte[] b, int off, int len) {
        try {
            os.write(b, off, len);
            os.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    public void sendFileHTTP(String fileName, PrintWriter filePrintWriter) {
        try (FileInputStream fileInputStream = new FileInputStream(fileName);) {
            File file = new File(fileName);
            String status = "200 OK", contentType = Files.probeContentType(file.toPath())/*"application/octet-stream"*/;
            write("HTTP/1.1 "+status+"\r\n");
            write("Server: Java HTTP Server: 1.0\r\n");
            write("Date: "+new Date()+"\r\n");
            write("Content-Type: "+contentType+"\r\n");
            write("Content-Length: "+file.length()+"\r\n");
            write("Content-Disposition: attachment\r\n");
            write("\r\n");
    
            filePrintWriter.println("Response:");
            filePrintWriter.println("HTTP/1.1 "+status+"\r\n");
            filePrintWriter.println("Server: Java HTTP Server: 1.0\r\n");
            filePrintWriter.println("Date: "+new Date()+"\r\n");
            filePrintWriter.println("Content-Type: "+contentType+"\r\n");
            filePrintWriter.println("Content-Length: "+file.length()+"\r\n");
            filePrintWriter.println("Content-Disposition: attachment\r\n");
            filePrintWriter.println("\r\n");
            filePrintWriter.close();
            
            byte[] buf = new byte[8<<10];   // no need to use buffered stream (default 8 KB) as we have to do that manually
            int count;
            while ((count = fileInputStream.read(buf))>0) {
                write(buf, 0, count);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    public void sendFile(String filePath) {
        try (FileInputStream fileInputStream = new FileInputStream(filePath)) {
            System.out.println("Start sending: "+filePath);
            write(new File(filePath).length()+"\n");
            byte[] buf = new byte[8<<10];   // no need to use buffered stream (default 8 KB) as we have to do that manually
            int count;
            while ((count = fileInputStream.read(buf))>0) {
                write(buf, 0, count);
//                try {
//                    Thread.sleep(10);
//                } catch (InterruptedException e) {
//                    e.printStackTrace();
//                }
            }
            System.out.println("End sending: "+filePath);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    public void receiveFile(String fileName) {
        try (FileOutputStream fileOutputStream = new FileOutputStream(fileName)) {
            long fileSize = Long.parseLong(readLine()), receivedSize = 0;
            System.out.println("Start receiving: "+fileName+" with size: "+fileSize);
            byte[] buf = new byte[8<<10];   // no need to use buffered stream (default 8 KB) as we have to do that manually
            int count;
            while (receivedSize<fileSize && (count = read(buf, 0, (int) Math.min(8<<10, fileSize-receivedSize)))>0) {
                fileOutputStream.write(buf, 0, count);
                receivedSize += count;
//                try {/
//                    Thread.sleep(10);
//                } catch (InterruptedException e) {
//                    e.printStackTrace();
//                }
            }
            System.out.println("End receiving: "+fileName);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    
    public void closeConnection() {
        try {
            System.out.println("Closing socket with port: "+socket.getPort());
            socket.close();
        } catch (Exception e) {
            System.out.println("Closing Error in network : ");
            e.printStackTrace(System.out);
        }
    }
}
