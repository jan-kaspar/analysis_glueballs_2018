import root;
import pad_layout;

string topDir = "../../";

string datasets[];
datasets.push("caption");
datasets.push("TOTEM2");
datasets.push("TOTEM4");

//----------------------------------------------------------------------------------------------------

void MakeCaption(string histName, string l)
{
	NewPad(false);
	label("\vbox{\hsize6cm\noindent {\it " + replace(histName, "_", "\_")+ "}\\ " + l + "}");
}

//----------------------------------------------------------------------------------------------------

for (int dsi : datasets.keys)
{
	NewRow();

	bool captionRow = (datasets[dsi] == "caption");

	NewPadLabel((captionRow) ? "" : datasets[dsi]);

	string f = topDir + datasets[dsi] + "/merged.root";

	real cen = 0.77;
	real tol = 0.124;

	string histName = "hmALLpiKen0_2D";
	if (captionRow)
	{
		MakeCaption(histName, "");
	} else {
		NewPad("$m_{11}$ or $m_{12}\ung{GeV}$", "$m_{22}$ or $m_{21}\ung{GeV}$", axesAbove=true);
		RootObject hist = RootGetObject(f, histName);
		draw(hist);
		draw((cen-tol, cen-tol)--(cen+tol, cen-tol)--(cen+tol, cen+tol)--(cen-tol, cen+tol)--cycle, black+1pt);
		limits((0.4, 0.4), (1.2, 1.2), Crop);
		//AttachLegend(replace(histName, "_", "\_"));
	}

	string histName = "hmALLpiKen0";
	if (captionRow)
	{
		MakeCaption(histName, "");
	} else {
		NewPad("$m_{11}$ or $m_{12}\ung{GeV}$");
		scale(Linear, Linear(true));
		draw(RootGetObject(f, histName), "vl,eb,", red);
		xlimits(0.4, 1.2, Crop);
		//AttachLegend(replace(histName, "_", "\_"));
		yaxis(XEquals(cen - tol, false), heavygreen);
		yaxis(XEquals(cen + tol, false), heavygreen);
	}

	string histName = "hmrhootherKen0";
	if (captionRow)
	{
		MakeCaption(histName, "cut: $m_{11}$ or $m_{12}$ matches $\rh$ mass");
	} else {
		NewPad("$m_{22}$ or $m_{21}\ung{GeV}$");
		scale(Linear, Linear(true));
		draw(RootGetObject(f, histName), "vl,eb", red);
		xlimits(0.4, 1.2, Crop);
		//AttachLegend(replace(histName, "_", "\_"));
		yaxis(XEquals(cen - tol, false), heavygreen);
		yaxis(XEquals(cen + tol, false), heavygreen);
	}
}

GShipout("mass_pair_dist_cut1", vSkip=0mm);
