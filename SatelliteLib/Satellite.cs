using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Text.Json;

namespace Sat
{
    public class Satellite
    {
        [DllImport("shell32.dll", SetLastError = true)]
        static extern IntPtr CommandLineToArgvW(
        [MarshalAs(UnmanagedType.LPWStr)] string p_cmdLine, out int p_numArgs);

        private static readonly string c_satelliteFileName = "satellite.json";

        private static void ReportFileStack(Stack<string> p_fileStack)
        {
            string fileStackReport = String.Join(Environment.NewLine, p_fileStack);
            Console.Out.WriteLine("FileStack:");
            Console.Out.WriteLine(fileStackReport);
        }

        public static string[] SeperateArgString(string p_args)
        {
            int argc;
            var argv = CommandLineToArgvW(p_args, out argc);
            if (argv == IntPtr.Zero)
            {
                throw new System.ComponentModel.Win32Exception();
            }

            var separatedArgs = new string[argc];

            try
            {
                for (var i = 0; i < separatedArgs.Length; i++)
                {
                    var arg = Marshal.ReadIntPtr(argv, i * IntPtr.Size);
                    separatedArgs[i] = Marshal.PtrToStringUni(arg);
                }
                return separatedArgs;
            }
            finally
            {
                Marshal.FreeHGlobal(argv);
            }
        }

        /// <summary>
        /// Performs substitution of certain symbols in the terms:
        /// - Replaces relative paths starting with './' with their absolute equivalent.
        /// </summary>
        /// <param name="p_term">The term in question</param>
        /// <returns>The evaluated version of the term.</returns>
        public static string EvaluateTerms(string p_terms)
        {
            string[] separatedArgs = SeperateArgString(p_terms);
            string[] evaluatedArgs = new string[separatedArgs.Length];

            for (int i = 0; i < evaluatedArgs.Length; i++)
            {
                string toEval = separatedArgs[i];
                if(!Path.IsPathRooted(toEval))
                {
                    toEval = Path.GetFullPath(toEval);
                }
                evaluatedArgs[i] = toEval;
            }

            return String.Join(' ', evaluatedArgs);
        }

        private static string ResolveExpression(string[] p_keys, string p_originalExpression, Dictionary<string, string> p_currentDict, Stack<string> p_fileStack)
        {
            if(p_keys.Length == 0)
            {
                Console.Out.WriteLine($"Satellite Error: ResolveExpression called with no keys. Invalid expression: \'{p_originalExpression}\'");
                Console.Out.WriteLine("Satellite: Aborting...");
                return null;
            }

            //Set the working directory to the path of the json file,
            //so that relative paths are correctly recognised.
            string dirName = Path.GetDirectoryName(p_fileStack.Peek());
            Directory.SetCurrentDirectory(dirName);

            string value;
            string leadingKey = p_keys.First();
            if (p_currentDict.TryGetValue(leadingKey, out value))
            {
                value = EvaluateTerms(value);

                string[] remainingKeys = p_keys.Skip(1).ToArray();
                if(remainingKeys.Length > 0)
                {
                    //Still components of the expression left to resolve
                    if(File.Exists(value) && Path.GetExtension(value) == ".json")
                    {
                        string satelliteFileContents = null;
                        Dictionary<string, string> lookupTable = null;
                        try
                        {
                            satelliteFileContents = File.ReadAllText(value);
                        }
                        catch (Exception err)
                        {
                            Console.Out.WriteLine($"Satellite Error: Reading from file \'{value}\' failed. {err.Message}");
                            ReportFileStack(p_fileStack);
                            Console.Out.WriteLine("Satellite: Aborting...");
                            return null;
                        }

                        try
                        {
                            lookupTable = JsonSerializer.Deserialize<Dictionary<string, string>>(satelliteFileContents);
                        }
                        catch (Exception err)
                        {
                            Console.Out.WriteLine($"Satellite Error: Failed to deserialize JSON from \'{value}\'. {err.Message}");
                            ReportFileStack(p_fileStack);
                            Console.Out.WriteLine("Satellite: Aborting...");
                            return null;
                        }

                        p_fileStack.Push(value);
                        return ResolveExpression(remainingKeys, p_originalExpression, lookupTable, p_fileStack);
                    }
                    else
                    {
                        Console.Out.WriteLine($"Satellite Error: File \'{value}\' does not exist or is not a .json file.");
                        ReportFileStack(p_fileStack);
                        Console.Out.WriteLine("Satellite: Aborting...");
                        return null;
                    }
                }
                else
                {
                    return value;
                }
            }
            else
            {
                Console.Out.WriteLine($"Satellite Error: Lookup failed for component \'{leadingKey}\' in given expression \'{p_originalExpression}\'");
                ReportFileStack(p_fileStack);
                Console.Out.WriteLine("Satellite: Aborting...");
                return null;
            }
        }

        private static string Relay(ISatellite p_satellite, string p_key)
        {
            //Working copy root
            string repoRoot = p_satellite.CheckoutRoot;
            string satelliteFile = Path.Combine(repoRoot, c_satelliteFileName);
            string satelliteFileContents = null;
            Dictionary<string, string> lookupTable = null;

            try
            {
                satelliteFileContents = File.ReadAllText(satelliteFile);
            }
            catch (Exception err)
            {
                Console.Out.WriteLine("Satellite Error: Reading from \'{0}\' failed. {1}", satelliteFile, err.Message);
                Console.Out.WriteLine("Satellite: Aborting...");
                return null;
            }

            try
            {
                lookupTable = JsonSerializer.Deserialize<Dictionary<string, string>>(satelliteFileContents);
            }
            catch (Exception err)
            {
                Console.Out.WriteLine("Satellite Error: Failed to deserialize JSON from \'{0}\'. {1}", satelliteFile, err.Message);
                Console.Out.WriteLine("Satellite: Aborting...");
                return null;
            }

            if (lookupTable != null)
            {
                string[] keys = p_key.Split(':');
                var fileStack = new Stack<string>();
                fileStack.Push(satelliteFile);
                return ResolveExpression(keys, p_key, lookupTable, fileStack);
            }

            return null;
        }

        public static string RelayGit(string p_originPath, string p_key)
        {
            string cwd = Directory.GetCurrentDirectory();
            ISatellite satellite = new SatelliteGit(p_originPath);
            if (!satellite.IsCheckout)
            {
                Console.Out.WriteLine("Satellite Error: Specified path \'{0}\' is not part of a git repository.", p_originPath);
                Console.Out.WriteLine("Satellite: Aborting...");
                return null;
            }

            return Relay(satellite, p_key);
        }
    }
}
