# Com.Gitusme.IO.Ports.SerialPort

Com.Gitusme.IO.Ports.SerialPort 是一款 Windows 串口通信组件，基于.Net Core 3.1 开发，支持 Console、WinFrom、Wpf 等 Windows 应用。集成方便快捷，能有效提高编程效率。

演示示例：

<code>
  
            Com.Gitusme.IO.Ports.SerialPort port1 = new Com.Gitusme.IO.Ports.SerialPort();
            port1.Settings = new Com.Gitusme.IO.Ports.SerialPortSettings
            {
                BaudRate = 115200,
                DataBits = 8,
                Parity = Com.Gitusme.IO.Ports.Parity.None,
                StopBits = 8,
                Synchronizable = false
            };
            port1.Open("COM1");

            Com.Gitusme.IO.Ports.SerialPort port2 = new Com.Gitusme.IO.Ports.SerialPort();
            port2.Settings = new Com.Gitusme.IO.Ports.SerialPortSettings
            {
                BaudRate = 115200,
                DataBits = 8,
                Parity = Com.Gitusme.IO.Ports.Parity.None,
                StopBits = 8,
                Synchronizable = false
            };
            port2.Open("COM2");

            string send = "Hello, gitusme!";
            byte[] data = System.Text.Encoding.UTF8.GetBytes(send);
            port1.Write(data);

            byte[] rec = new byte[data.Length + 10000];
            int read = port2.Read(rec);
            string receive = System.Text.Encoding.UTF8.GetString(rec, 0, read);

            port1.Close();
            port2.Close();

</code>
