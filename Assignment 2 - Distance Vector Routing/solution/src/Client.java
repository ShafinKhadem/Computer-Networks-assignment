import java.util.ArrayList;
import java.util.Random;
import java.util.UUID;

//Work needed
public class Client {
    public static void main(String[] args) throws InterruptedException {
        NetworkUtility networkUtility = new NetworkUtility("127.0.0.1", 4444);
        System.out.println("Connected to server");
        /**
         * Tasks
         */

        /*
        1. Receive EndDevice configuration from server
        2. Receive active client list from server
        3. for(int i=0;i<100;i++)
        4. {
        5.      Generate a random message
        6.      Assign a random receiver from active client list
        7.      if(i==20)
        8.      {
        9.            Send the message and recipient IP address to server and a special request "SHOW_ROUTE"
        10.           Display routing path, hop count and routing table of each router [You need to receive
                            all the required info from the server in response to "SHOW_ROUTE" request]
        11.     }
        12.     else
        13.     {
        14.           Simply send the message and recipient IP address to server.
        15.     }
        16.     If server can successfully send the message, client will get an acknowledgement along with hop count
                    Otherwise, client will get a failure message [dropped packet]
        17. }
        18. Report average number of hops and drop rate
        */

        int sumHop = 0, cntDrop = 0;
        EndDevice config = (EndDevice) networkUtility.read();
        System.out.println(config);
        ArrayList<IPAddress> activeClients = (ArrayList<IPAddress>) networkUtility.read();
        System.out.println("Active clients: " + activeClients);
        if (activeClients.size() > 1) activeClients.remove(config.getIpAddress());
        for (int i = 0; i < 100; i++) {
            String message = UUID.randomUUID().toString();
            IPAddress destIp = activeClients.get(new Random().nextInt(activeClients.size()));
            Packet packet = new Packet(message, "", config.getIpAddress(), destIp);
            if (i==20) {
                packet.setSpecialMessage("SHOW_ROUTE");
                System.out.println("Packet destination: " + packet.getDestinationIP());
            }
            networkUtility.write(packet);
            if (i==20) {
                ArrayList<Integer> routingPath = (ArrayList<Integer>) networkUtility.read();
                ArrayList<String> routingTables = (ArrayList<String>) networkUtility.read();
                System.out.println("Routing path: " + routingPath);
                System.out.println("Routing table: " + routingTables);
            }
            int acknowledgement = (Integer) networkUtility.read();
            if (acknowledgement==-1) {
                if (i==20) System.out.println("Message dropped");
                ++cntDrop;
            } else {
                if (i==20) System.out.println(acknowledgement);
                sumHop += acknowledgement;
            }
        }
        System.out.println("Average number of hops: " + sumHop/(100.0-cntDrop));
        System.out.println("Drop rate: " + cntDrop/100.0);
    }
}
