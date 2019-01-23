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

	NewPad("$m_{\ph\ph}\ung{GeV}$");
	draw(RootGetObject(f, "hm4PHImass"), "eb", red);
	xlimits(1.9, 4., Crop);
	AttachLegend("hm4PHImass");

	NewPad("number of identified K's");
	draw(RootGetObject(f, "hnKcurves"), "vl", red);
	AttachLegend("hnKcurves");

	NewPad("$m_{\ph\ph}\ung{GeV}$");
	draw(RootGetObject(f, "hm4PHImass34curves"), "eb", red);
	xlimits(1.9, 4., Crop);
	AttachLegend("hm4PHImass34curves");
}
