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

		real cen = 1.02;
		real tol = 0.02;
		if (suffixes[sfi] == "_PhiCutStrict")
			tol = 0.01;

		string histName = "hmALLkKen0_2D";
		if (captionRow)
		{
			MakeCaption(histName, "");
		} else {
			NewPad("$m_{11}$ or $m_{12}\ung{GeV}$", "$m_{22}$ or $m_{21}\ung{GeV}$", axesAbove=true);
			RootObject hist = RootGetObject(f, histName);
			draw(hist);
			draw((cen-tol, cen-tol)--(cen+tol, cen-tol)--(cen+tol, cen+tol)--(cen-tol, cen+tol)--cycle, black+1pt);
			limits((0.9, 0.9), (1.3, 1.3), Crop);
			//AttachLegend(replace(histName, "_", "\_"));
		}

		string histName = "hmALLkKen0";
		if (captionRow)
		{
			MakeCaption(histName, "");
		} else {
			NewPad("$m_{11}$ or $m_{12}\ung{GeV}$");
			draw(RootGetObject(f, histName), "vl,eb", red);
			xlimits(0.9, 1.3, Crop);
			//AttachLegend(replace(histName, "_", "\_"));
			yaxis(XEquals(cen - tol, false), heavygreen);
			yaxis(XEquals(cen + tol, false), heavygreen);
		}

		string histName = "hmphiotherKen0";
		if (captionRow)
		{
			MakeCaption(histName, "cut: $m_{11}$ or $m_{12}$ matches $\ph$ mass");
		} else {
			NewPad("$m_{22}$ or $m_{21}\ung{GeV}$");
			draw(RootGetObject(f, histName), "vl,eb", red);
			xlimits(0.9, 1.3, Crop);
			//AttachLegend(replace(histName, "_", "\_"));
			yaxis(XEquals(cen - tol, false), heavygreen);
			yaxis(XEquals(cen + tol, false), heavygreen);
		}
	}

	GShipout("mass_pair_dist" + suffixes[sfi], vSkip=0mm);
}
