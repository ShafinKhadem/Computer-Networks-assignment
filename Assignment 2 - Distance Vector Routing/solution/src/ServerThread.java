

import java.util.ArrayList;
import java.util.Random;

public class ServerThread implements Runnable {

    NetworkUtility networkUtility;
    EndDevice endDevice;

    ServerThread(NetworkUtility networkUtility, EndDevice endDevice) {
        this.networkUtility = networkUtility;
        this.endDevice = endDevice;
        System.out.println("Server Ready for client " + NetworkLayerServer.clientCount);
        NetworkLayerServer.clientCount++;
        new Thread(this).start();
    }

    @Override
    public void run() {
        /**
         * Synchronize actions with client.
         */

        /*
        Tasks:
        Send endDevice and activeClients to client.
        1. Upon receiving a packet and recipient, call deliverPacket(packet)
        2. If the packet contains "SHOW_ROUTE" request, then fetch the required information
                and send back to client
        3. Either send acknowledgement with number of hops or send failure message back to client
        Manage activeClients.
        */

        networkUtility.write(endDevice);
        networkUtility.write(NetworkLayerServer.activeClients);
//        while (true) {
        for (int i = 0; i < 100; i++) {
            Packet packet = (Packet) networkUtility.read();
            ArrayList<Integer> routingPath = deliverPacket(packet);
            if (packet.getSpecialMessage().equals("SHOW_ROUTE")) {
                ArrayList<String> routingTables = new ArrayList<>();
                for (Router r :
                        NetworkLayerServer.routers) {
                    routingTables.add(r.strRoutingTable());
                }
                networkUtility.write(routingPath);
                networkUtility.write(routingTables);
            }
            networkUtility.write(packet.hopcount);
        }
    }


    public ArrayList<Integer> deliverPacket(Packet p) {

        /*
        1. Find the router s which has an interface
                such that the interface and source end device have same network address.
        2. Find the router d which has an interface
                such that the interface and destination end device have same network address.
        3. Implement forwarding, i.e., s forwards to its gateway router x considering d as the destination.
                similarly, x forwards to the next gateway router y considering d as the destination,
                and eventually the packet reaches to destination router d.

            3(a) If, while forwarding, any gateway x, found from routingTable of router r is in down state[x.state==FALSE]
                    (i) Drop packet
                    (ii) Update the entry with distance Constants.INFTY
                    (iii) Block NetworkLayerServer.stateChanger.t
                    (iv) Apply DVR starting from router r.
                    (v) Resume NetworkLayerServer.stateChanger.t

            3(b) If, while forwarding, a router x receives the packet from router y,
                    but routingTableEntry shows Constants.INFTY distance from x to y,
                    (i) Update the entry with distance 1
                    (ii) Block NetworkLayerServer.stateChanger.t
                    (iii) Apply DVR starting from router x.
                    (iv) Resume NetworkLayerServer.stateChanger.t

        4. If 3(a) occurs at any stage, packet will be dropped,
            otherwise successfully sent to the destination router
        */

//        NetworkLayerServer.blockStateChanger();
        ArrayList<Integer> routingPath = new ArrayList<>();
        Router s = NetworkLayerServer.routerMap.get(NetworkLayerServer.deviceIDtoRouterID.get(NetworkLayerServer.endDeviceMap.get(p.getSourceIP()).getDeviceID()));
        Router d = NetworkLayerServer.routerMap.get(NetworkLayerServer.deviceIDtoRouterID.get(NetworkLayerServer.endDeviceMap.get(p.getDestinationIP()).getDeviceID()));
//        System.out.println("source router: " + s.getRouterId());
//        System.out.println("destination router: " + d.getRouterId());
        while (true) {
            ++p.hopcount;
//            System.out.println(s.strRoutingTable());
//            System.out.println(s.getState());
            routingPath.add(s.getRouterId());
            if (s.getRouterId() == d.getRouterId()) break;
            Router x = null;
            for (RoutingTableEntry rte :
                    s.getRoutingTable()) {
                if (rte.getRouterId() == d.getRouterId()) {
                    x = NetworkLayerServer.routerMap.get(rte.getGatewayRouterId());
                    break;
                }
            }
            if (x==null) {
                System.out.println("The router currently with the packet has gone down");
                p.hopcount = -1;
                break;
            }
            if (!x.getState()) {
                p.hopcount = -1;
                for (RoutingTableEntry rte :
                        s.getRoutingTable()) {
                    if (rte.getRouterId() == x.getRouterId()) {
                        rte.setDistance(Constants.INFINITY);
                    }
                }
                NetworkLayerServer.DVR(s.getRouterId());
                break;
            } else {
                for (RoutingTableEntry rte :
                        x.getRoutingTable()) {
                    if (rte.getRouterId() == s.getRouterId()) {
                        if (rte.getDistance() == Constants.INFINITY) {
                            rte.setDistance(1);
                            NetworkLayerServer.DVR(x.getRouterId());
                        }
                    }
                }
                s = x;
            }
        }
//        NetworkLayerServer.blockStateChanger();
        return routingPath;
    }

    @Override
    public boolean equals(Object obj) {
        return super.equals(obj); //To change body of generated methods, choose Tools | Templates.
    }
}
