import root;
import pad_layout;

string topDir = "../../";

string datasets[];
datasets.push("caption");
datasets.push("TOTEM2");
datasets.push("TOTEM4");

string suffixes[];
suffixes.push("");
suffixes.push("_PhiCutStrict");

xTicksDef = LeftTicks(0.5, 0.1);

//----------------------------------------------------------------------------------------------------

void MakeCaption(string histName, string l)
{
	NewPad(false);
	label("\vbox{\hsize6cm\noindent {\it " + replace(histName, "_", "\_")+ "}\\ " + l + "}");
}

//----------------------------------------------------------------------------------------------------

for (int sfi : suffixes.keys)
{
	for (int dsi : datasets.keys)
	{
		NewRow();

		bool captionRow = (datasets[dsi] == "caption");

		NewPadLabel((captionRow) ? "" : datasets[dsi]);

		string f = topDir + datasets[dsi] + "/merged.root";

		int rebin = 5;

		string histName = "hnKcurves" + suffixes[sfi];
		if (captionRow)
		{
			MakeCaption(histName, "number of kaons identified with $\d E/\d x$");
		} else {
			NewPad("number of identified K's");
			currentpad.xTicks = LeftTicks(1., 1.);
			draw(shift(-0.5, 0.), RootGetObject(f, histName), "vl", red);
			AttachLegend();
		}

		string histName = "hm4PHImass" + suffixes[sfi];
		if (captionRow)
		{
			MakeCaption(histName, "no PID required");
		} else {
			NewPad("$m_{\ph\ph}\ung{GeV}$");
			RootObject hist = RootGetObject(f, histName);
			hist.vExec("Rebin", rebin);
			draw(hist, "eb,lE", red);
			xlimits(1.9, 3.2, Crop);
			AttachLegend();
		}


		string histName = "hm4PHImass1234curves" + suffixes[sfi];
		if (captionRow)
		{
			MakeCaption(histName, "at least 1 identified kaon");
		} else {
			NewPad("$m_{\ph\ph}\ung{GeV}$");
			RootObject hist = RootGetObject(f, histName);
			hist.vExec("Rebin", rebin);
			draw(hist, "eb,lE", red);
			xlimits(1.9, 3.2, Crop);
			AttachLegend();
		}


		string histName = "hm4PHImass234curves" + suffixes[sfi];
		if (captionRow)
		{
			MakeCaption(histName, "at least 2 identified kaons");
		} else {
			NewPad("$m_{\ph\ph}\ung{GeV}$");
			RootObject hist = RootGetObject(f, histName);
			hist.vExec("Rebin", rebin);
			draw(hist, "eb,lE", red);
			xlimits(1.9, 3.2, Crop);
			AttachLegend();
		}


		string histName = "hm4PHImass34curves" + suffixes[sfi];
		if (captionRow)
		{
			MakeCaption(histName, "at least 3 identified kaons");
		} else {
			NewPad("$m_{\ph\ph}\ung{GeV}$");
			RootObject hist = RootGetObject(f, histName);
			hist.vExec("Rebin", rebin);
			draw(hist, "eb,lE", red);
			xlimits(1.9, 3.2, Crop);
			AttachLegend();
		}


		string histName = "hm4PHImass34curves2SS" + suffixes[sfi];
		if (captionRow)
		{
			MakeCaption(histName, "at least 3 identified kaons OR two identified kaons of the same charge sign");
		} else {
			NewPad("$m_{\ph\ph}\ung{GeV}$");
			RootObject hist = RootGetObject(f, histName);
			hist.vExec("Rebin", rebin);
			draw(hist, "eb,lE", red);
			xlimits(1.9, 3.2, Crop);
			AttachLegend();
			//yaxis(XEquals(2.25, false), heavygreen);
		}

			/*
			string histName = "hm4PHImass4curves" + suffixes[sfi];
			NewPad("$m_{\ph\ph}\ung{GeV}$");
			RootObject hist = RootGetObject(f, histName);
			hist.vExec("Rebin", rebin);
			draw(hist, "eb,lE", red);
			xlimits(1.9, 3.2, Crop);
			AttachLegend(replace(histName, "_", "\_"));
			*/


		string histName = "hm4PHImassPiVeto" + suffixes[sfi];
		if (captionRow)
		{
			MakeCaption(histName, "none of the tracks identified as pion");
		} else {
			NewPad("$m_{\ph\ph}\ung{GeV}$");
			RootObject hist = RootGetObject(f, histName);
			hist.vExec("Rebin", rebin);
			draw(hist, "eb,lE", red);
			xlimits(1.9, 3.2, Crop);
			AttachLegend();
			//yaxis(XEquals(2.25, false), heavygreen);
		}


		string histName = "hm4PHImass1234curvesPiVeto" + suffixes[sfi];
		if (captionRow)
		{
			MakeCaption(histName, "none of the tracks identified as pion AND at least one track identified as kaon");
		} else {
			NewPad("$m_{\ph\ph}\ung{GeV}$");
			RootObject hist = RootGetObject(f, histName);
			hist.vExec("Rebin", rebin);
			draw(hist, "eb,lE", red);
			xlimits(1.9, 3.2, Crop);
			AttachLegend(replace(histName, "_", "\_"));
			//yaxis(XEquals(2.25, false), heavygreen);
		}
	}

	GShipout("mass_dist" + suffixes[sfi], vSkip=0mm);
}
