import root;
import pad_layout;

string topDir = "../";

string datasets[];
datasets.push("TOTEM2");
datasets.push("TOTEM4");

//----------------------------------------------------------------------------------------------------

for (int dsi : datasets.keys)
{
	NewRow();

	NewPadLabel(datasets[dsi]);

	string f = topDir + datasets[dsi] + "/merged.root";

	real cen = 1.02;
	real tol = 0.02;

	string histName = "hmALLkKen0";
	NewPad("$m_{11}$ or $m_{12}\ung{GeV}$");
	draw(RootGetObject(f, histName), "vl,eb", red);
	xlimits(0.9, 1.3, Crop);
	AttachLegend(histName);
	yaxis(XEquals(cen - tol, false), heavygreen);
	yaxis(XEquals(cen + tol, false), heavygreen);

	string histName = "hmphiotherKen0";
	NewPad("$m_{22}$ or $m_{21}\ung{GeV}$");
	draw(RootGetObject(f, histName), "vl,eb", red);
	xlimits(0.9, 1.3, Crop);
	AttachLegend(histName);
	yaxis(XEquals(cen - tol, false), heavygreen);
	yaxis(XEquals(cen + tol, false), heavygreen);
}
