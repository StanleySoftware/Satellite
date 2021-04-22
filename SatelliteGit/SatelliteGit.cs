using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using LibGit2Sharp;

namespace Sat
{
    public class SatelliteGit : ISatellite
    {
        bool m_isCheckout = false;
        string m_checkoutRoot = null;

        public SatelliteGit(string p_currentWorkingDirectory)
        {
            string gitDirPath = Repository.Discover(p_currentWorkingDirectory);         
            if(String.IsNullOrEmpty(gitDirPath))
            {
                m_isCheckout = false;
                return;
            }

            using (Repository repo = new Repository(gitDirPath))
            {
                m_checkoutRoot = repo.Info.WorkingDirectory;
            }
            m_isCheckout = (m_checkoutRoot != null);
        }

        bool ISatellite.IsCheckout { get { return m_isCheckout; } }
        string ISatellite.CheckoutRoot { get { return m_checkoutRoot; } }
    }
}
