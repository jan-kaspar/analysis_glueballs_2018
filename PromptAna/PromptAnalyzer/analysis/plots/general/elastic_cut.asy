import root;
import pad_layout;

string topDir = "../../";

string datasets[];

datasets.push("TOTEM2");
//datasets.push("TOTEM2/319176");
//datasets.push("TOTEM2/319256");
//datasets.push("TOTEM2/319262");
//datasets.push("TOTEM2/319263");
//datasets.push("TOTEM2/319265");
//datasets.push("TOTEM2/319268");
//datasets.push("TOTEM2/319300");
//datasets.push("TOTEM2/319311");

/*
datasets.push("TOTEM4");
//datasets.push("TOTEM4/319176");
//datasets.push("TOTEM4/319256");
datasets.push("TOTEM4/319262");
//datasets.push("TOTEM4/319263");
//datasets.push("TOTEM4/319265");
//datasets.push("TOTEM4/319268");
//datasets.push("TOTEM4/319300");
//datasets.push("TOTEM4/319311");
*/

//----------------------------------------------------------------------------------------------------

for (int dsi : datasets.keys)
{
	NewRow();

	NewPadLabel(datasets[dsi]);

	string f = topDir + datasets[dsi] + "/merged.root";

	string histName = "hthxEla";
	NewPad("$\th^{*L}_x + \th^{*R}_x\ung{\mu rad}$");
	draw(scale(1e6, 1.), RootGetObject(f, histName), "vl", red);
	xlimits(-250., +250., Crop);
	AttachLegend(histName);
	yaxis(XEquals(-45, false), heavygreen);
	yaxis(XEquals(+45, false), heavygreen);

	string histName = "hthyEla";
	NewPad("$\th^{*L}_y + \th^{*R}_y\ung{\mu rad}$");
	draw(scale(1e6, 1.), RootGetObject(f, histName), "vl", red);
	xlimits(-250., +250., Crop);
	AttachLegend(histName);
	yaxis(XEquals(-15, false), heavygreen);
	yaxis(XEquals(+15, false), heavygreen);
}
