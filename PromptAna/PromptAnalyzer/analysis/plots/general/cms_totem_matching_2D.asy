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

TH2_palette = Gradient(white, blue, heavygreen, yellow, red);

//----------------------------------------------------------------------------------------------------

for (int dsi : datasets.keys)
{
	NewRow();

	NewPadLabel(datasets[dsi]);

	string f = topDir + datasets[dsi] + "/merged.root";

	string histName = "h2DIMdpx4trk";
	NewPad("$p_x^{\rm CMS}\ung{GeV}$", "$p_x^{\rm TOTEM}\ung{GeV}$", axesAbove=true);
	RootObject hist = RootGetObject(f, histName);
	hist.vExec("Rebin2D", 2, 2);
	draw(hist);
	limits((-2, -2), (+2, +2), Crop);
	AttachLegend(histName);

	string histName = "h2DIMdpy4trk";
	NewPad("$p_y^{\rm CMS}\ung{GeV}$", "$p_y^{\rm TOTEM}\ung{GeV}$", axesAbove=true);
	RootObject hist = RootGetObject(f, histName);
	hist.vExec("Rebin2D", 2, 2);
	draw(hist);
	limits((-2, -2), (+2, +2), Crop);
	AttachLegend(histName);
}
