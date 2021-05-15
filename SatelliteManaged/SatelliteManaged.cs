using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Runtime.InteropServices;
using System.Text;

namespace Sat
{
    // === Native ===

    using StrHnd = UInt32;

    internal static class StrHndUtil
    {
        public static string AsString(StrHnd p_strhnd)
        {
            UInt32 len = NativeMethods.sat_length_from_strhnd(p_strhnd);
            StringBuilder strBuild = new StringBuilder((Int32)len);
            NativeMethods.sat_copy_from_strhnd(p_strhnd, len + 1, strBuild);
            return strBuild.ToString();
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    internal struct SatError
    {
        public StrHnd m_errorMessage;
        public Int32 m_errorCode;
    }
    [StructLayout(LayoutKind.Sequential)]
    internal struct SatCheckoutInfo
    {
        public bool m_isCheckout;
        public StrHnd m_checkoutRoot;
    }
    internal static class NativeMethods
    {

        [DllImport("SatelliteC.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool sat_init();

        [DllImport("SatelliteC.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool sat_shutdown();

        [DllImport("SatelliteC.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern SatError sat_create(UInt32 p_vcs, ref IntPtr p_out_satellite);

        [DllImport("SatelliteC.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void sat_load(IntPtr p_satellite);

        [DllImport("SatelliteC.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void sat_unload(IntPtr p_satellite);

        [DllImport("SatelliteC.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern SatError sat_checkout_info(IntPtr p_satellite, [MarshalAs(UnmanagedType.LPStr)] string p_targetPath, ref SatCheckoutInfo p_out_checkoutInfo);

        [DllImport("SatelliteC.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern SatError sat_relay(IntPtr p_satellite, [MarshalAs(UnmanagedType.LPStr)] string p_originPath, [MarshalAs(UnmanagedType.LPStr)] string p_query, ref StrHnd p_out_string);

        [DllImport("SatelliteC.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern void sat_release(ref IntPtr p_satellite);


        [DllImport("SatelliteC.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern UInt32 sat_length_from_strhnd(StrHnd p_strhnd);

        [DllImport("SatelliteC.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern SatError sat_copy_from_strhnd(StrHnd p_strhnd, UInt32 p_size, StringBuilder p_out_string);

        [DllImport("SatelliteC.dll", CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool sat_clear_string_table();

    }

    // === Managed ===

    public enum VCSType : uint
    {
        GIT = 0
    }

    public enum SatelliteErrorType : Int32
    {
        UNSPECIFIED = -1,
        SUCCESS = 0,
        NOINIT,
        NOTACHECKOUT,
        BARECHECKOUT,
        FILEREAD,
        JSONPARSE,
        EMPTYQUERY,
        SEPARATEARGFAILURE,
        EMPTYVALUE,
        FILEDOESNOTEXIST,
        FILENOTJSON,
        INVALIDSTRHND,
        STRCOPYFAILURE
    };

    public class SatelliteError
    {
        public SatelliteError() { }
        internal SatelliteError(SatError p_error)
        {
            m_errorCode = (SatelliteErrorType)(p_error.m_errorCode);
            string mess = StrHndUtil.AsString(p_error.m_errorMessage);
            if (mess != null)
            {
                m_errorMessage = mess;
            }
        }

        public SatelliteErrorType m_errorCode = SatelliteErrorType.SUCCESS;
        public string m_errorMessage = string.Empty;
    };

    public class SatelliteCheckoutInfo
    {
        public SatelliteCheckoutInfo() { }
        internal SatelliteCheckoutInfo(SatCheckoutInfo p_ci)
        {
            m_isCheckout = p_ci.m_isCheckout;
            string mess = StrHndUtil.AsString(p_ci.m_checkoutRoot);
            if (mess != null)
            {
                m_checkoutRoot = mess;
            }
        }

        public bool m_isCheckout = false;
        public string m_checkoutRoot = null;
    };

    public class Satellite
    {
        public static Dictionary<string, VCSType> VCSMap = new Dictionary<string, VCSType>(StringComparer.InvariantCultureIgnoreCase)
        {
            { "git", VCSType.GIT }
        };

        public Satellite(VCSType p_vcsType)
        {
            m_vcsType = p_vcsType;
        }

        public static bool Init()
        {
            return NativeMethods.sat_init();
        }

        public static bool Shutdown()
        {
            return NativeMethods.sat_shutdown();
        }

        /// <summary>
        /// Creates a native satellite object and sets p_out_satellite to point to it. Loads any underlying VCS libraries.
        /// </summary>
        /// <param name="p_out_satellite">Out parameter for ptr to native satellite object. IntPtr.Zero if an error is encountered.</param>
        /// <returns>Returns an encountered error or a succesful state.</returns>
        protected SatelliteError Create(out IntPtr p_out_satellite)
        {
            SatelliteError creationError = new SatelliteError();
            p_out_satellite = IntPtr.Zero;
            SatError err = NativeMethods.sat_create((uint)m_vcsType, ref p_out_satellite);
            if (err.m_errorCode != 0)
            {
                creationError = new SatelliteError(err);
                return creationError;
            }
            NativeMethods.sat_load(p_out_satellite);
            return creationError;
        }

        /// <summary>
        /// Destroys a non-null native ptr to a satellite object. Shuts down any underlying VCS libraries.
        /// </summary>
        /// <param name="p_out_satellite">The ptr to the satellite object.</param>
        protected void Destroy(ref IntPtr p_out_satellite)
        {
            Debug.Assert(p_out_satellite != IntPtr.Zero);
            NativeMethods.sat_unload(p_out_satellite);
            NativeMethods.sat_release(ref p_out_satellite);
        }

        /// <summary>
        /// Retrieves checkout information for the VCS checkout with contains the path 'p_targetPath'.
        /// </summary>
        /// <param name="p_targetPath">The target path to inspect.</param>
        /// <param name="p_out_checkoutInfo">The output SatelliteCheckoutInfo object.</param>
        /// <returns>Returns an encountered error or a succesful state.</returns>
        public SatelliteError CheckoutInfo(string p_targetPath, out SatelliteCheckoutInfo p_out_checkoutInfo)
        {
            p_out_checkoutInfo = null;
            IntPtr satellite = IntPtr.Zero;

            SatelliteError err = Create(out satellite);
            if(err.m_errorCode != 0)
            {
                return err;
            }

            SatCheckoutInfo ci = new SatCheckoutInfo();
            SatError errCheckout = NativeMethods.sat_checkout_info(satellite, p_targetPath, ref ci);
            if(errCheckout.m_errorCode == 0)
            {
                p_out_checkoutInfo = new SatelliteCheckoutInfo(ci);
            }

            err = new SatelliteError(errCheckout);

            Destroy(ref satellite);

            return err;
        }

        /// <summary>
        /// Invokes a relay command from the target path 'p_originPath' with the query 'p_query'.
        /// Sets output parameter 'p_out_string' with the lookup response if successful.
        /// Queries are colon separated keys, e.g. key1:key2:key3.
        /// A colon represents a lookup of another satellite.json file, using the result
        /// of the preceding key to identify which json file to load next. If the preceding
        /// key does not produce a path to a satellite.json file, then the query is ill-formed.
        /// The last key in the sequence does not need to resolve to a satellite.json file.
        /// </summary>
        /// <param name="p_originPath">Lookup always begins from the root of the checkout, but this path is used by VCS to determine the checkout.</param>
        /// <param name="p_query">The query for relay to resolve against.</param>
        /// <param name="p_out_string">The resulting string looked up from the query</param>
        /// <returns>Returns an encountered error or a succesful state.</returns>
        public SatelliteError Relay(string p_originPath, string p_query, ref string p_out_string)
        {
            p_out_string = null;
            IntPtr satellite = IntPtr.Zero;

            SatelliteError err = Create(out satellite);
            if (err.m_errorCode != 0)
            {
                return err;
            }

            StrHnd cstring = new StrHnd();
            SatError errRelay = NativeMethods.sat_relay(satellite, p_originPath, p_query, ref cstring);
            if (errRelay.m_errorCode == 0)
            {
                p_out_string = StrHndUtil.AsString(cstring);
            }

            err = new SatelliteError(errRelay);

            Destroy(ref satellite);

            return err;
        }

        VCSType m_vcsType;
    }
}
