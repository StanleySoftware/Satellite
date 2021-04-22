using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.CommandLine;
using System.CommandLine.Invocation;
using System.Configuration;

namespace Sat
{
    class SatelliteApp
    {
        private static readonly Dictionary<string, Type> VCSTypes = new Dictionary<string, Type>()
        {
            { "git", typeof(SatelliteGit)}
        };

        private static void PassThrough(Stream p_instream, Stream p_outstream)
        {
            byte[] buffer = new byte[4096];
            while (true)
            {
                int len;
                while ((len = p_instream.Read(buffer, 0, buffer.Length)) > 0)
                {
                    p_outstream.Write(buffer, 0, len);
                    p_outstream.Flush();
                }
            }
        }

        private static void OutputReader(object p_process)
        {
            var process = (System.Diagnostics.Process)p_process;
            PassThrough(process.StandardOutput.BaseStream, Console.OpenStandardOutput());
        }

        private static void ErrorReader(object p_process)
        {
            var process = (System.Diagnostics.Process)p_process;
            PassThrough(process.StandardError.BaseStream, Console.OpenStandardError());
        }

        private static void InputReader(object p_process)
        {
            var process = (System.Diagnostics.Process)p_process;
            PassThrough(Console.OpenStandardInput(), process.StandardInput.BaseStream);
        }

        private static int Invoke(string p_executableTarget, string p_args, string p_workingDirectory)
        {
            if (p_executableTarget != null)
            {
                // Fires up a new process to run inside this one
                var processStartInfo = new ProcessStartInfo();
                processStartInfo.UseShellExecute = false;
                processStartInfo.RedirectStandardError = true;
                processStartInfo.RedirectStandardInput = true;
                processStartInfo.RedirectStandardOutput = true;
                processStartInfo.FileName = p_executableTarget;
                processStartInfo.WorkingDirectory = p_workingDirectory;

                processStartInfo.Arguments = string.Join(' ', p_args);

                using (System.Diagnostics.Process exeProcess = new System.Diagnostics.Process())
                {
                    exeProcess.StartInfo = processStartInfo;

                    // Depending on your application you may either prioritize the IO or the exact opposite
                    var outputThread = new Thread(OutputReader) { Name = "Satellite Output", Priority = ThreadPriority.Normal };
                    var errorThread = new Thread(ErrorReader) { Name = "Satellite Error", Priority = ThreadPriority.Normal };
                    var inputThread = new Thread(InputReader) { Name = "Satellite Input", Priority = ThreadPriority.Normal };

                    // Set as background threads (will automatically stop when application ends)
                    outputThread.IsBackground = true;
                    errorThread.IsBackground = true;
                    inputThread.IsBackground = true;

                    // Signal to end the application
                    ManualResetEvent stopApp = new ManualResetEvent(false);

                    // Enables the exited event and set the stopApp signal on exited
                    exeProcess.EnableRaisingEvents = true;
                    exeProcess.Exited += (e, sender) => { stopApp.Set(); };

                    try
                    {
                        exeProcess.Start();

                        // Start the IO threads
                        outputThread.Start(exeProcess);
                        errorThread.Start(exeProcess);
                        inputThread.Start(exeProcess);
                    }
                    catch (Exception err)
                    {
                        Console.Out.WriteLine($"Satellite Error: Unable to start process \'{p_executableTarget}\'. {err.Message}");
                        Console.Out.WriteLine("Satellite: Aborting...");
                    }

                    // Wait for the child app to stop
                    stopApp.WaitOne();
                    return exeProcess.ExitCode;
                }
            }

            return 0;
        }

        private static int InvokeCommand(string query, string[] args, string vcs, string targetDir)
        {
            //Ensure we have !invoke at the end
            string[] keys = query.Split(':');
            if (keys.Last() != "!invoke")
            {
                List<string> fullKeys = new List<string>(keys.Append("!invoke"));
                query = String.Join(':', fullKeys);
            }

            string oldWorkingDirectory = Directory.GetCurrentDirectory();
            string invokePrefix = Satellite.RelayGit(targetDir, query);
            if(invokePrefix == null)
            {
                return -1;
            }

            string joinedArgs = String.Join(' ', args);
            string fullCommand = $"{invokePrefix} {joinedArgs}";

            string[] separatedArgs = Satellite.SeperateArgString(fullCommand);

            string executableName = separatedArgs.First();
            string fullArgs = "";
            if (separatedArgs.Length > 1)
            {
                fullArgs = String.Join(' ', separatedArgs.Skip(1).Take(separatedArgs.Length - 1));
            }
            if (executableName != null)
            {
                return Invoke(executableName, fullArgs, oldWorkingDirectory);
            }
            return -1;
        }

        public static int Main(string[] p_args)
        {
            // Create a root command with some options
            var rootCommand = new RootCommand
            {
                new Argument<string>(
                    "query",
                    description: "The query to resolve against the satellite.json files - the target for invocation."),
                new Option<string[]>(
                    "--args",
                    getDefaultValue: () => new string[] {},
                    description: "List of arguments to pass to the invoked target."),
                new Option<string>(
                    "--vcs",
                    getDefaultValue: () => "git",
                    description: $"The name for the VCS system to use. Valid values are [{String.Join(", ",VCSTypes.Keys)}]"),
                new Option<string>(
                    "--targetDir",
                    getDefaultValue: () => Directory.GetCurrentDirectory(),
                    description: "The path from which to invoke the query. By default uses the current working directory.")
            };

            rootCommand.Description = "Invoking satellite detects the root directory of the checkout containing the working directory." +
                "It then reads a satellite.json file in that root directory and performs a lookup of the query." +
                "It will resolve the query by traversing satellite.jsons in multiple directories if they are referenced." +
                "Once the full query has been resolved, takes the string returned from the query, and invokes it as a process.";

            // Note that the parameters of the handler method are matched according to the names of the options
            rootCommand.Handler = CommandHandler.Create<string, string[], string, string>(InvokeCommand);

            // Parse the incoming args and invoke the handler
            return rootCommand.InvokeAsync(p_args).Result;
        }
    }
}
