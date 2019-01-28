import root;
import pad_layout;

string topDir = "../../";

string datasets[];
datasets.push("TOTEM2");
datasets.push("TOTEM4");

TH2_x_min = 0;
TH2_x_max = 2;

TH2_y_min = 0;
TH2_y_max = 15;

TH2_palette = Gradient(blue, heavygreen, yellow, red);

//----------------------------------------------------------------------------------------------------

real piu3, ku3, kd3;

void Curves(real p)
{
	piu3 = 3.49965e+00-2.88850e-01*atan(p-1.51050e+00) + exp(3.03012e+00-1.53867e+01*p);

	kd3 = 1.46882e+00-9.31222e-04*p + exp(2.12237e+00-4.34300e+00*p);
	ku3 = 4.53163e+00-5.89828e-01*p + exp(3.16431e+00-5.38011e+00*p);
}

real PionMax(real p)
{
	Curves(p);

	return piu3;
}

real KaonMin(real p)
{
	Curves(p);

	//return kd3;
	return max(kd3, piu3);
}

real KaonMax(real p)
{
	Curves(p);

	//return max(ku3, piu3);
	return ku3;
}

//----------------------------------------------------------------------------------------------------

for (int dsi : datasets.keys)
{
	NewRow();

	NewPadLabel(datasets[dsi]);

	string f = topDir + datasets[dsi] + "/merged.root";

	//string histName = "hdedxAllPIX";
	//string histName = "hdedx2trk";
	string histName = "hdedx4trk";

	NewPad("$p\ung{GeV}$", "$\d E/\d x$");
	scale(Linear, Linear, Log);
	RootObject hist = RootGetObject(f, histName);
	//hist.vExec("Rebin2D", 2, 2);
	draw(hist);

	draw(graph(KaonMin, 0.05, 2.), black);
	draw(graph(KaonMax, 0.05, 2.), black);

	draw(graph(PionMax, 0.05, 2.), magenta+dashed);

	limits((0, 0), (2, 15), Crop);

	AddToLegend("Kaon", black);
	AddToLegend("Pion", magenta+dashed);
	AttachLegend(histName);
}

GShipout("dEdx", vSkip=0mm);
