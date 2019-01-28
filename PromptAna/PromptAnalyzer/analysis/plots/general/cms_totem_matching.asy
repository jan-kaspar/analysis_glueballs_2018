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

datasets.push("TOTEM4");
//datasets.push("TOTEM4/319176");
//datasets.push("TOTEM4/319256");
//datasets.push("TOTEM4/319262");
//datasets.push("TOTEM4/319263");
//datasets.push("TOTEM4/319265");
//datasets.push("TOTEM4/319268");
//datasets.push("TOTEM4/319300");
//datasets.push("TOTEM4/319311");

//----------------------------------------------------------------------------------------------------

for (int dsi : datasets.keys)
{
	NewRow();

	NewPadLabel(datasets[dsi]);

	string f = topDir + datasets[dsi] + "/merged.root";

	string histName = "hdpx4trk";
	NewPad("$p_x^{\rm CMS} + p_x^{\rm TOTEM}\ung{GeV}$");
	draw(RootGetObject(f, histName), "vl", red);
	AttachLegend(histName);
	yaxis(XEquals(-0.15, false), heavygreen);
	yaxis(XEquals(+0.15, false), heavygreen);

	string histName = "hdpy4trk";
	NewPad("$p_y^{\rm CMS} + p_y^{\rm TOTEM}\ung{GeV}$");
	draw(RootGetObject(f, histName), "vl", red);
	AttachLegend(histName);
	yaxis(XEquals(-0.06, false), heavygreen);
	yaxis(XEquals(+0.06, false), heavygreen);
}
