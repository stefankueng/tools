using System;
using System.Net;
using System.Net.Sockets;

namespace MonitorClient
{
    internal class MonitorClient
    {
        private static void Main(string[] args)
        {
            if (args.Length == 0)
            {
                Console.Out.Write("specify an ip address to connect to and ON or OFF as the second parameter");
                return;
            }
            if (args.Length == 1)
            {
                Console.Out.Write("specify ON or OFF as the second parameter");
                return;
            }
            IPAddress ipadr = IPAddress.Parse(args[0]);
            IPEndPoint localEndPoint = new IPEndPoint(ipadr, 8000);
            Socket newsock = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            newsock.Connect(localEndPoint);
            System.Text.ASCIIEncoding encoding = new System.Text.ASCIIEncoding();
            newsock.Send(encoding.GetBytes("Monitor" + args[1]));
            System.Threading.Thread.Sleep(1000);
        }
    }
}
