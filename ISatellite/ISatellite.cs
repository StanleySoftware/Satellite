﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Sat
{
    public interface ISatellite
    {
        bool IsCheckout{ get; }
        string CheckoutRoot { get; }
    }
}
