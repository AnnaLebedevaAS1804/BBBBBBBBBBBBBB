using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace LAB1
{
    public partial class Form1 : Form
    {
        Process ChildProcess = null;
        readonly EventWaitHandle EventStart = new EventWaitHandle(false, EventResetMode.AutoReset, "event_start");//(занято,автосброс,название)
        readonly EventWaitHandle EventStop = new EventWaitHandle(false, EventResetMode.AutoReset, "event_stop");
        readonly EventWaitHandle EventQuit = new EventWaitHandle(false, EventResetMode.AutoReset, "event_quit");
        readonly EventWaitHandle EventConfirm = new EventWaitHandle(false, EventResetMode.AutoReset, "event_confirm");
       
        
        public Form1()
        {
            InitializeComponent();
        }


        private Boolean consoleIsOpen()
        {
            if (ChildProcess == null || ChildProcess.HasExited)
            {
                listBox1.Items.Clear();
                return false;
            }
            return true;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            int maxThreads = comboBox1.SelectedIndex + 1;  // количество потоков
            for (int i = 0; i < maxThreads; ++i)
            {
                if (consoleIsOpen())
                {
                    EventStart.Set();
                    EventConfirm.WaitOne();

                    listBox1.Items.Add("Поток номер - " + (listBox1.Items.Count - 1).ToString());
                }
                else
                {
                    ChildProcess = Process.Start("Lab1_Lebedeva.exe");
                    listBox1.Items.Add("Все потоки");
                    listBox1.Items.Add("Главный поток");
                    break;
                }
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if (!consoleIsOpen()) return;

            if (listBox1.Items.Count - 1 == 0)
            {
                EventQuit.Set();
                EventConfirm.WaitOne();
                listBox1.Items.RemoveAt(0);
            }
            else
            {
                EventStop.Set();
                EventConfirm.WaitOne();
                listBox1.Items.RemoveAt(listBox1.Items.Count - 1);
            }
        }

        private void Form1_Load(object sender, EventArgs e)
        {
            comboBox1.SelectedIndex = 0;
        }

        private void Form1_FormClosing(object sender, FormClosingEventArgs e)
        {
            if (consoleIsOpen())
            {
                EventQuit.Set();
                EventConfirm.WaitOne();
            }
        }
    }
}
