import root;
import pad_layout;

string topDir = "../../";

string datasets[];
datasets.push("caption");
datasets.push("TOTEM2");
datasets.push("TOTEM4");

string cuts[], c_tags[];
cuts.push("cut2"); c_tags.push("SIG1");
cuts.push("cut4"); c_tags.push("SIG2");

xTicksDef = LeftTicks(0.5, 0.1);

//----------------------------------------------------------------------------------------------------

void MakeCaption(string histName, string l)
{
	NewPad(false);
	label("\vbox{\hsize6cm\noindent {\it " + replace(histName, "_", "\_")+ "}\\ " + l + "}");
}

//----------------------------------------------------------------------------------------------------

for (int ci : cuts.keys)
{
	for (int dsi : datasets.keys)
	{
		NewRow();

		bool captionRow = (datasets[dsi] == "caption");

		NewPadLabel((captionRow) ? "" : datasets[dsi]);

		string f = topDir + datasets[dsi] + "/merged.root";

		int rebin = 4;

		string histName = "hnPIcurves" + c_tags[ci];
		if (captionRow)
		{
			MakeCaption(histName, "number of pions identified with $\d E/\d x$");
		} else {
			NewPad("number of identified $\pi$'s");
			scale(Linear, Linear(true));
			currentpad.xTicks = LeftTicks(1., 1.);
			draw(shift(-0.5, 0), RootGetObject(f, histName), "vl", red);
			//AttachLegend(replace(histName, "_", "\_"), NW, NW);
		}

		string histName = "hm4" + c_tags[ci] + "mass";
		if (captionRow)
		{
			MakeCaption(histName, "no PID required");
		} else {
			NewPad("$m_{\rh\rh}\ung{GeV}$");
			scale(Linear, Linear(true));
			RootObject hist = RootGetObject(f, histName);
			hist.vExec("Rebin", rebin);
			draw(hist, "eb,lE", red);
			xlimits(1.3, 3.2, Crop);
			AttachLegend();
		}

		string histName = "hm4" + c_tags[ci] + "mass1234curves";
		if (captionRow)
		{
			MakeCaption(histName, "at least 1 identified pion");
		} else {
			NewPad("$m_{\rh\rh}\ung{GeV}$");
			scale(Linear, Linear(true));
			RootObject hist = RootGetObject(f, histName);
			hist.vExec("Rebin", rebin);
			draw(hist, "eb,lE", red);
			xlimits(1.3, 3.2, Crop);
			AttachLegend();
		}

		string histName = "hm4" + c_tags[ci] + "mass234curves";
		if (captionRow)
		{
			MakeCaption(histName, "at least 2 identified pions");
		} else {
			NewPad("$m_{\rh\rh}\ung{GeV}$");
			scale(Linear, Linear(true));
			RootObject hist = RootGetObject(f, histName);
			hist.vExec("Rebin", rebin);
			draw(hist, "eb,lE", red);
			xlimits(1.3, 3.2, Crop);
			AttachLegend();
		}

		string histName = "hm4" + c_tags[ci] + "mass4curves";
		if (captionRow)
		{
			MakeCaption(histName, "at least 3 identified pions");
		} else {
			NewPad("$m_{\rh\rh}\ung{GeV}$");
			scale(Linear, Linear(true));
			RootObject hist = RootGetObject(f, histName);
			hist.vExec("Rebin", rebin);
			draw(hist, "eb,lE", red);
			xlimits(1.3, 3.2, Crop);
			AttachLegend();
		}

		string histName = "hm4" + c_tags[ci] + "mass34curves";
		if (captionRow)
		{
			MakeCaption(histName, "at least 4 identified pions");
		} else {
			NewPad("$m_{\rh\rh}\ung{GeV}$");
			scale(Linear, Linear(true));
			RootObject hist = RootGetObject(f, histName);
			hist.vExec("Rebin", rebin);
			draw(hist, "eb,lE", red);
			xlimits(1.3, 3.2, Crop);
			AttachLegend();
		}
	}

	GShipout("mass_dist_" + cuts[ci], vSkip=0mm);
}
