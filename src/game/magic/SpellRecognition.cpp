/*
 * Copyright 2014 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "game/magic/SpellRecognition.h"

#include <iostream>
#include <map>
#include <string>

#include <boost/lexical_cast.hpp>

#include "core/Config.h"
#include "game/Equipment.h"
#include "game/Player.h"
#include "game/magic/Spell.h"
#include "game/spell/Cheat.h"
#include "graphics/Math.h"
#include "graphics/particle/ParticleEffects.h"
#include "math/GtxFunctions.h"
#include "scene/GameSound.h"
#include "input/Input.h"
#include "io/log/Logger.h"

static const size_t RESAMPLE_POINT_COUNT = 16;
static std::vector<Vec2f> plist;
bool bPrecastSpell = false;

static void handleRuneDetection(Rune);

typedef struct RunePattern{
	Rune runeId;
	CheatRune cheatId;
	const float templateVec[RESAMPLE_POINT_COUNT*2];
} RunePattern;

const RunePattern patternData[] = {
	{RUNE_AAM,         CheatRune_AAM,        {-0.406863, 3.55691e-08, -0.352575, -0.000796302, -0.298287, -0.00159264, -0.243999, -0.00238898, -0.189711, -0.00318531, -0.135423, -0.00398165, -0.0813088, -0.00168211, -0.0270406, -0.00212547, 0.0270931, -0.000178901, 0.0813813, -0.000975239, 0.135544, 0.00132359, 0.189778, 0.00185872, 0.243994, 0.00282609, 0.298241, 0.00303733, 0.352444, 0.00432858, 0.406732, 0.00353225}},
	{RUNE_CETRIUS,     CheatRune_None,       {-0.233993, -0.233993, -0.217921, -0.14811, -0.20253, -0.0621044, -0.187684, 0.0239989, -0.172928, 0.110118, -0.153763, 0.195269, -0.128413, 0.278399, -0.071131, 0.214764, -0.0176266, 0.145693, 0.0327868, 0.0743304, 0.0787166, 3.24377e-05, 0.122874, -0.0753605, 0.163996, -0.152384, 0.244681, -0.145587, 0.329504, -0.124655, 0.413431, -0.100412}},
	{RUNE_COMUNICATUM, CheatRune_COMUNICATUM,{-0.251249, -0.251249, -0.122347, -0.235005, 0.00742411, -0.227857, 0.136539, -0.211705, 0.245721, -0.17862, 0.250825, -0.0485621, 0.215859, 0.04701, 0.0871818, 0.0286367, -0.0412028, 0.00731656, -0.169167, -0.0159844, -0.246719, 0.0317272, -0.256362, 0.161491, -0.158801, 0.210519, -0.0288826, 0.216263, 0.100888, 0.226173, 0.230292, 0.239845}},
	{RUNE_COMUNICATUM, CheatRune_COMUNICATUM,{-0.202617, -0.202617, -0.0937695, -0.241691, 0.0154167, -0.279808, 0.117928, -0.272459, 0.189382, -0.182611, 0.206886, -0.0875682, 0.0943307, -0.0615039, -0.0192146, -0.0395988, -0.132801, -0.0178771, -0.214294, 0.0261326, -0.189211, 0.139013, -0.164145, 0.251907, -0.0714789, 0.277165, 0.0422011, 0.257421, 0.155073, 0.232233, 0.266313, 0.201863}},
	{RUNE_COSUM,       CheatRune_None,       {-0.427752, 3.73953e-08, -0.303833, -0.016456, -0.180154, -0.0355467, -0.0574657, -0.059828, 0.0662134, -0.0789187, 0.189892, -0.0980095, 0.2889, -0.0917332, 0.297412, 0.0329809, 0.317748, 0.15631, 0.198361, 0.178545, 0.0744006, 0.194915, -0.0480871, 0.209499, -0.0763165, 0.0876472, -0.0966376, -0.0357998, -0.113105, -0.159795, -0.129576, -0.283811}},
	{RUNE_FOLGORA,     CheatRune_None,       {-0.280985, 0.280985, -0.254717, 0.209885, -0.228332, 0.138815, -0.196263, 0.0701079, -0.163012, 0.00199843, -0.132045, -0.0672198, -0.0965087, -0.134197, -0.0568138, -0.198851, -0.016355, -0.219302, 0.0327206, -0.161503, 0.0892181, -0.110939, 0.144442, -0.058924, 0.202146, -0.00974753, 0.26098, 0.0379811, 0.319388, 0.0861421, 0.376137, 0.134768}},
	{RUNE_FOLGORA,     CheatRune_None,       {-0.371207, -3.2452e-08, -0.308355, -0.0336509, -0.245502, -0.0673018, -0.18265, -0.100953, -0.120311, -0.135488, -0.0599354, -0.173405, -6.44143e-05, -0.21211, 0.0285416, -0.179729, 0.0482941, -0.111226, 0.0773742, -0.0461717, 0.106938, 0.0187038, 0.137591, 0.0830549, 0.169837, 0.14664, 0.202083, 0.210225, 0.23841, 0.271416, 0.278956, 0.329996}},
	{RUNE_FRIDD,       CheatRune_None,       {-0.263747, 0.263747, -0.246351, 0.181179, -0.228909, 0.0987596, -0.211496, 0.0163113, -0.196596, -0.0666888, -0.181701, -0.149649, -0.119296, -0.179189, -0.0352722, -0.17187, 0.0485015, -0.161998, 0.1321, -0.150688, 0.215731, -0.139795, 0.236475, -0.0756196, 0.228012, 0.00833973, 0.218187, 0.0920924, 0.208372, 0.175776, 0.195989, 0.259292}},
	{RUNE_FRIDD,       CheatRune_None,       {-0.316676, -2.76847e-08, -0.29836, -0.0753849, -0.278489, -0.15035, -0.220999, -0.166798, -0.14445, -0.154204, -0.0679455, -0.141342, 0.008531, -0.128315, 0.0850075, -0.115288, 0.161484, -0.102261, 0.218258, -0.0757222, 0.196137, -0.00136471, 0.174743, 0.0732029, 0.153769, 0.147892, 0.131783, 0.222288, 0.109663, 0.296646, 0.0875419, 0.371003}},
	{RUNE_KAOM,        CheatRune_KAOM,       {0.238542, -0.238542, 0.165789, -0.25424, 0.0914744, -0.257334, 0.0174505, -0.251828, -0.0524577, -0.226954, -0.110919, -0.182075, -0.144185, -0.116446, -0.153409, -0.0426626, -0.153491, 0.0317125, -0.143843, 0.105358, -0.120027, 0.175239, -0.0688968, 0.227685, -0.000157757, 0.254709, 0.0733364, 0.265508, 0.147581, 0.267409, 0.213213, 0.242461}},
	{RUNE_KAOM,        CheatRune_KAOM,       {-1.49059e-08, -0.341007, -0.055689, -0.304577, -0.109136, -0.265088, -0.154489, -0.2168, -0.169465, -0.153517, -0.162879, -0.0874171, -0.148066, -0.0225406, -0.126126, 0.0400592, -0.0905056, 0.0950337, -0.0425896, 0.140602, 0.0139074, 0.175765, 0.0773483, 0.190456, 0.14358, 0.196918, 0.209497, 0.197327, 0.274703, 0.184038, 0.339909, 0.170749}},
	{RUNE_MEGA,        CheatRune_MEGA,       {-1.77753e-08, 0.406651, 0.000722002, 0.352414, -0.00223022, 0.29826, -0.0042161, 0.244091, -0.00349408, 0.189853, -0.00277206, 0.135616, -0.00205004, 0.081379, -0.00132802, 0.0271418, -0.000606001, -0.0270955, 0.000116018, -0.0813327, 0.000838038, -0.13557, 0.00156006, -0.189807, 0.00228208, -0.244044, 0.0030041, -0.298282, 0.00372612, -0.352519, 0.00444813, -0.406756}},
	{RUNE_MORTE,       CheatRune_None,       {-0.429701, 3.75656e-08, -0.367934, -0.00536982, -0.306408, -0.0130936, -0.244547, -0.0176234, -0.182803, -0.0236, -0.121012, -0.0288625, -0.0592465, -0.0343996, 0.00264966, -0.0385033, 0.0645458, -0.0426069, 0.126442, -0.0467106, 0.188338, -0.0508142, 0.250234, -0.0549178, 0.263705, -0.00371871, 0.267808, 0.0581774, 0.271912, 0.120074, 0.276016, 0.181969}},
	{RUNE_MOVIS,       CheatRune_None,       {-0.256308, -0.256308, -0.140236, -0.230513, -0.0234548, -0.207407, 0.093326, -0.1843, 0.210498, -0.163529, 0.286471, -0.145073, 0.178934, -0.0943635, 0.0721865, -0.0417829, -0.0338913, 0.0120899, -0.137649, 0.0704062, -0.239684, 0.131689, -0.235871, 0.177743, -0.11909, 0.20085, -0.00143684, 0.218831, 0.115404, 0.241589, 0.230801, 0.270079}},
	{RUNE_MOVIS,       CheatRune_None,       {-0.214546, -0.214546, -0.11142, -0.24846, -0.00822501, -0.28215, 0.0960254, -0.312375, 0.0927196, -0.219974, 0.0405064, -0.124796, -0.016402, -0.0324336, -0.0757611, 0.0584595, -0.140797, 0.144983, -0.2135, 0.225599, -0.17024, 0.237371, -0.0647373, 0.211997, 0.0403648, 0.18482, 0.144941, 0.15573, 0.249162, 0.12535, 0.351908, 0.0904245}},
	{RUNE_NHI,         CheatRune_None,       {0.406535, 0, 0.352425, 0.00398672, 0.298273, 0.00717418, 0.244049, 0.00602735, 0.189821, 0.00601199, 0.135599, 0.0040428, 0.0813777, 0.00207361, 0.0271427, 0.0010251, -0.027106, 0.000910264, -0.0813276, -0.00105893, -0.135549, -0.00302812, -0.189771, -0.0049973, -0.244015, -0.0042892, -0.298249, -0.00520153, -0.352491, -0.00535388, -0.406712, -0.00732306}},
	{RUNE_RHAA,        CheatRune_None,       {-1.77788e-08, -0.406731, -0.000925706, -0.352476, 0.00191616, -0.298302, 0.000990475, -0.244048, 6.47866e-05, -0.189793, 0.00203191, -0.1356, 0.00198097, -0.0813644, 0.00105528, -0.0271098, 0.000129593, 0.0271446, 0.00178398, 0.0813511, 0.0020456, 0.135584, 0.00111991, 0.189838, 0.000194222, 0.244093, -0.000731467, 0.298347, -0.00531006, 0.352409, -0.00634564, 0.406658}},
	{RUNE_SPACIUM,     CheatRune_SPACIUM,    {0.224402, -0.224403, 0.112905, -0.222977, 0.00138167, -0.210695, -0.110142, -0.198412, -0.221849, -0.188823, -0.224201, -0.0843875, -0.215559, 0.0274124, -0.205018, 0.139065, -0.195341, 0.250573, -0.0836011, 0.24098, 0.0279223, 0.228697, 0.139648, 0.219064, 0.205786, 0.173262, 0.193504, 0.0617382, 0.181221, -0.0497852, 0.168939, -0.161309}},
	{RUNE_STREGUM,     CheatRune_STREGUM,    {-0.251419, 0.251419, -0.238397, 0.137918, -0.223906, 0.024577, -0.208604, -0.0886747, -0.186679, -0.200723, -0.146381, -0.245231, -0.0900501, -0.145877, -0.0339385, -0.0463219, 0.0156698, 0.0565589, 0.0770651, 0.152823, 0.148966, 0.241311, 0.185449, 0.196921, 0.206391, 0.084649, 0.228355, -0.0273987, 0.249234, -0.139668, 0.268244, -0.252281}},
	{RUNE_STREGUM,     CheatRune_STREGUM,    {-0.24671, -2.15681e-08, -0.21325, -0.111364, -0.178064, -0.222163, -0.139056, -0.328503, -0.110097, -0.216, -0.0764593, -0.10469, -0.0454939, 0.00737322, -0.0160376, 0.119862, 0.0132834, 0.232386, 0.0424551, 0.344949, 0.0794682, 0.324925, 0.116209, 0.214608, 0.15124, 0.103751, 0.180486, -0.00879261, 0.209461, -0.121406, 0.232567, -0.234936}},
	{RUNE_TAAR,        CheatRune_None,       {-0.419064, 3.66358e-08, -0.35377, -0.0191347, -0.288871, -0.0397515, -0.22292, -0.0567791, -0.156893, -0.0735299, -0.0905373, -0.088836, -0.0371017, -0.0804989, -0.0176553, -0.0152448, 0.0013876, 0.0501245, 0.0292273, 0.0992415, 0.0945685, 0.080244, 0.160596, 0.0634932, 0.226623, 0.0467424, 0.29265, 0.0299915, 0.358283, 0.0118155, 0.423477, -0.00787751}},
	{RUNE_TAAR,        CheatRune_None,       {-0.285078, -0.285078, -0.2087, -0.262189, -0.132322, -0.239301, -0.0559436, -0.216412, 0.0201222, -0.192626, 0.0577703, -0.149808, 0.0157186, -0.0823582, -0.0324268, -0.0188009, -0.0805722, 0.0447563, -0.107584, 0.119447, -0.0564365, 0.157956, 0.0194543, 0.182202, 0.0964549, 0.2029, 0.173456, 0.223598, 0.249855, 0.246413, 0.326233, 0.269301}},
	{RUNE_TEMPUS,      CheatRune_None,       {-0.247866, 0.247866, -0.274883, 0.154023, -0.304788, 0.0609336, -0.284576, -0.014341, -0.194362, -0.0519492, -0.101395, -0.0822191, -0.06144, -0.000139952, -0.018947, 0.0879017, 0.0327147, 0.151375, 0.124926, 0.118627, 0.215798, 0.0831995, 0.201656, -0.0056078, 0.171808, -0.0985368, 0.154829, -0.185427, 0.245903, -0.220605, 0.340622, -0.2451}},
	{RUNE_TEMPUS,      CheatRune_None,       {-0.340618, -2.97778e-08, -0.332463, -0.100702, -0.270841, -0.147299, -0.169869, -0.143184, -0.068894, -0.138624, -0.0686651, -0.0585138, -0.0947098, 0.0391488, -0.118583, 0.137347, -0.0414495, 0.175487, 0.0576438, 0.195257, 0.150187, 0.19697, 0.173826, 0.0987912, 0.194689, -0.000104362, 0.209801, -0.0986935, 0.309975, -0.0852101, 0.409971, -0.07067}},
	{RUNE_TEMPUS,      CheatRune_None,       {-0.363208, -3.17526e-08, -0.356863, -0.0839577, -0.291031, -0.098908, -0.207021, -0.0932622, -0.122971, -0.0882435, -0.0985983, -0.0341853, -0.110466, 0.0491667, -0.0842251, 0.115371, -0.00164831, 0.130454, 0.0825483, 0.131203, 0.1666, 0.128549, 0.19926, 0.0624845, 0.211779, -0.0207752, 0.242362, -0.0796528, 0.325602, -0.0675095, 0.407881, -0.050734}},
	{RUNE_TERA,        CheatRune_None,       {-0.284487, 0.284487, -0.237073, 0.198654, -0.200059, 0.107692, -0.168264, 0.0147108, -0.136545, -0.0782986, -0.106957, -0.171944, -0.0724971, -0.263887, -0.0405652, -0.272633, -0.0144615, -0.177904, 0.0137583, -0.0838264, 0.0469977, 0.00861471, 0.0849178, 0.099229, 0.143101, 0.141165, 0.23359, 0.10293, 0.324681, 0.066119, 0.413862, 0.0248915}},
	{RUNE_TERA,        CheatRune_None,       {-0.351683, -3.07451e-08, -0.262622, -0.0448705, -0.173562, -0.0897424, -0.0847998, -0.135201, 0.00345072, -0.181627, 0.0914013, -0.22861, 0.181766, -0.270699, 0.180195, -0.215699, 0.128163, -0.130708, 0.0742009, -0.0468727, 0.0248586, 0.0397908, -0.0231276, 0.127154, -0.0618932, 0.218568, 0.00154474, 0.27617, 0.0921695, 0.317792, 0.179939, 0.364554}},
	{RUNE_VISTA,       CheatRune_None,       {-1.73222e-08, -0.396286, 0.026759, -0.331625, 0.0535181, -0.266964, 0.0797942, -0.202107, 0.105585, -0.137054, 0.131376, -0.0720014, 0.157167, -0.0069484, 0.179037, 0.0593291, 0.139932, 0.102414, 0.0791804, 0.137146, 0.0133175, 0.158807, -0.0548795, 0.174499, -0.123077, 0.190191, -0.192616, 0.195331, -0.26257, 0.19721, -0.332526, 0.198058}},
	{RUNE_VISTA,       CheatRune_None,       {-0.289352, -0.289352, -0.214278, -0.249253, -0.138511, -0.210501, -0.0622029, -0.172801, 0.0142484, -0.1354, 0.0913456, -0.0993422, 0.167421, -0.0612164, 0.2429, -0.0218831, 0.289171, 0.0240043, 0.21904, 0.0711926, 0.143407, 0.10996, 0.065736, 0.144765, -0.0125748, 0.177989, -0.0925875, 0.207008, -0.172499, 0.236289, -0.251264, 0.268541}},
	{RUNE_VITAE,       CheatRune_None,       {-0.432712, -3.78289e-08, -0.368542, 0.0100343, -0.304397, 0.0200679, -0.23978, 0.0270436, -0.175164, 0.0340193, -0.110548, 0.0409949, -0.0459312, 0.0479706, 0.0185589, 0.0559481, 0.0827653, 0.0659963, 0.146873, 0.076682, 0.211436, 0.0840423, 0.232321, 0.0369242, 0.237107, -0.0278911, 0.242362, -0.0926611, 0.249338, -0.157277, 0.256313, -0.221894}},
	{RUNE_YOK,         CheatRune_None,       {-0.228799, -0.228799, -0.229124, -0.143065, -0.228599, -0.0572984, -0.229778, 0.0282924, -0.226116, 0.114037, -0.201412, 0.180229, -0.116041, 0.173005, -0.0313713, 0.159531, 0.0524263, 0.141107, 0.137247, 0.130296, 0.222655, 0.122661, 0.229443, 0.0465105, 0.21786, -0.03801, 0.214198, -0.123754, 0.210536, -0.209499, 0.206874, -0.295243}},
	//cheat runes
	{RUNE_NONE,        CheatRune_O,          {-0.244939, -2.14133e-08, -0.175087, -0.0936803, -0.108218, -0.189682, -0.0398833, -0.284478, 0.0397049, -0.301055, 0.116336, -0.212855, 0.187782, -0.120207, 0.259299, -0.0276306, 0.267037, 0.0720403, 0.187406, 0.157432, 0.0958803, 0.230249, 0.00673398, 0.30518, -0.057525, 0.26626, -0.116933, 0.165479, -0.174153, 0.063446, -0.243442, -0.0304968}},
	{RUNE_NONE,        CheatRune_M,          {-0.215315, 0.215315, -0.256778, 0.0894619, -0.292707, -0.0380664, -0.308651, -0.160513, -0.220431, -0.0616437, -0.130517, 0.0354568, -0.0378845, 0.130161, 0.0563746, 0.223125, 0.0970702, 0.15942, 0.101146, 0.0269601, 0.110791, -0.105222, 0.1283, -0.236554, 0.168668, -0.25412, 0.219157, -0.131605, 0.267839, -0.00832419, 0.312937, 0.116147}},
	{RUNE_NONE,        CheatRune_A,          {-0.209786, 0.209786, -0.189898, 0.0960312, -0.179998, -0.0189637, -0.175393, -0.134385, -0.16912, -0.249736, -0.131428, -0.304166, -0.0377584, -0.236615, 0.0558331, -0.168904, 0.146486, -0.0974651, 0.232879, -0.0207851, 0.3156, 0.0597768, 0.291369, 0.112573, 0.180325, 0.144419, 0.0691271, 0.175698, -0.0430402, 0.203336, -0.155197, 0.229399}},
	{RUNE_NONE,        CheatRune_Passwall,   {-0.289667, -0.289667, -0.214366, -0.274349, -0.137671, -0.267462, -0.13429, -0.199423, -0.144037, -0.123042, -0.128948, -0.0664706, -0.0524509, -0.0577147, 0.0236197, -0.0460123, 0.0369311, 0.0110536, 0.0199789, 0.0861664, 0.0391433, 0.133905, 0.115505, 0.143809, 0.191439, 0.156506, 0.241631, 0.189222, 0.224922, 0.264323, 0.20826, 0.339155}},
	{RUNE_NONE,        CheatRune_S,          {-1.83945e-08, -0.420818, -0.040272, -0.340756, -0.0807459, -0.260807, -0.1203, -0.180381, -0.160564, -0.100306, -0.191118, -0.0229457, -0.102977, -0.00709497, -0.0138069, 0.00184914, 0.0757488, 0.00546394, 0.165356, 0.00533772, 0.203379, 0.0349474, 0.15731, 0.111819, 0.10619, 0.18544, 0.0536552, 0.258052, -0.000389298, 0.329515, -0.0514652, 0.400683}},
	{RUNE_NONE,        CheatRune_F,          {-1.86609e-08, 0.426911, -0.0119266, 0.359661, -0.0213794, 0.292051, -0.0340329, 0.224906, -0.0497453, 0.158445, -0.0668371, 0.0922913, -0.087832, 0.0272823, -0.106763, -0.0383678, -0.124319, -0.104282, -0.129281, -0.166174, -0.0621371, -0.178828, 0.00500697, -0.191481, 0.0721511, -0.204135, 0.138802, -0.218997, 0.205575, -0.233315, 0.272719, -0.245968}},
	{RUNE_NONE,        CheatRune_R,          {-1.64069e-08, 0.375346, -0.0481884, 0.259702, -0.0969324, 0.144282, -0.145397, 0.028754, -0.19295, -0.0871565, -0.240991, -0.20285, -0.237027, -0.275337, -0.115523, -0.246186, 0.00484521, -0.211543, 0.125518, -0.177975, 0.214796, -0.130555, 0.144211, -0.027193, 0.0594114, 0.0649754, 0.0537612, 0.138814, 0.17705, 0.160095, 0.297417, 0.186828}},
	{RUNE_NONE,        CheatRune_U,          {-0.287281, -0.287281, -0.264315, -0.205137, -0.243693, -0.122356, -0.223129, -0.0395603, -0.204654, 0.0436706, -0.164671, 0.107325, -0.0824874, 0.130035, 0.000835449, 0.148176, 0.0848119, 0.163206, 0.168926, 0.177298, 0.253256, 0.187318, 0.237756, 0.103773, 0.215616, 0.0214623, 0.194189, -0.0610679, 0.171518, -0.143271, 0.143321, -0.223592}},
	{RUNE_NONE,        CheatRune_W,          {-0.293906, -0.293906, -0.279124, -0.169286, -0.254633, -0.0461736, -0.222021, 0.0751103, -0.173778, 0.124085, -0.108172, 0.0170005, -0.0439734, -0.0909789, 0.0155092, -0.201614, 0.0462158, -0.127414, 0.0729612, -0.00465981, 0.100322, 0.117961, 0.125687, 0.240983, 0.166076, 0.256404, 0.22208, 0.143947, 0.280468, 0.0327231, 0.346289, -0.0741801}},
	{RUNE_NONE,        CheatRune_P,          {-2.01501e-08, 0.460981, -0.0297858, 0.356797, -0.0570443, 0.251925, -0.0860852, 0.147539, -0.11489, 0.04309, -0.140167, -0.0622301, -0.156849, -0.169208, -0.159535, -0.277118, -0.0747341, -0.271421, 0.0280116, -0.237058, 0.12733, -0.19417, 0.222767, -0.14308, 0.234187, -0.0810437, 0.149282, -0.0137255, 0.0668855, 0.056612, -0.00937242, 0.132109}},
	{RUNE_NONE,        CheatRune_X,          {-0.27344, -0.27344, -0.160406, -0.193033, -0.0466921, -0.113742, 0.0714926, -0.0411251, 0.189724, 0.0313896, 0.30711, 0.105299, 0.239109, 0.15703, 0.103252, 0.184845, -0.0324161, 0.213676, -0.167262, 0.246209, -0.234385, 0.232819, -0.152527, 0.120834, -0.0740046, 0.00650596, 0.00139067, -0.109935, 0.0759024, -0.226927, 0.153153, -0.340405}},
	{RUNE_NONE,        CheatRune_ChangeSkin, {-1.49197e-08, 0.341323, -0.0085214, 0.0159196, -0.0113943, -0.309557, -0.0141996, -0.230771, -0.0147941, 0.0948322, -0.0165644, 0.280429, -0.0251842, -0.0449578, -0.0549769, -0.369166, -0.0531952, -0.151843, -0.0211547, 0.172138, 0.000960831, 0.325282, 0.0204198, 0.000361564, 0.0277899, -0.325158, 0.038398, -0.25788, 0.0569141, 0.0671923, 0.0755021, 0.391855}},
	{RUNE_NONE,        CheatRune_26,         {-1.87115e-08, -0.428068, -0.0113982, -0.362936, -0.0193344, -0.297269, -0.0299325, -0.23202, -0.0453698, -0.167728, -0.058455, -0.102958, -0.0704111, -0.0379977, -0.0839203, 0.0266468, -0.0975666, 0.0912481, -0.110748, 0.155939, -0.0728618, 0.1879, -0.00818132, 0.201467, 0.0559631, 0.217533, 0.120005, 0.234079, 0.184047, 0.250624, 0.248165, 0.263541}}
};

class RuneRecognitionAlt {
	static const size_t RUNE_PATTERN_COUNT = ARRAY_SIZE(patternData);
	static constexpr float PATTERN_SIMILARITY_THRESHOLD = 1.6;

	float VecPlist[RESAMPLE_POINT_COUNT*2];
	Vec2f resampledPlist[RESAMPLE_POINT_COUNT];

	float optimalCosDistance(const float* templ);
	int findMatchingPattern();
	void resampleInput();
	void vectorizeInput();
	static void callRuneHandlers(int index);

public:
	void analyze();

};

/*!
 * Call appropriate handlers for a rune at patternData's index
 */
void RuneRecognitionAlt::callRuneHandlers(int index) {
	if(patternData[index].runeId != RUNE_NONE) {
		handleRuneDetection(patternData[index].runeId);
	} 
	if(patternData[index].cheatId != CheatRune_None) {
		handleCheatRuneDetection(patternData[index].cheatId);
	}
}

/*!
 * Resample the input while keeping key points contained in m_indices
 * Based on the resample function of $1 Unistroke Recognizer
 * https://depts.washington.edu/aimgroup/proj/dollar/
 */
void RuneRecognitionAlt::resampleInput() {
	std::cout << "PLIST: ";
	for (size_t i = 0; i < plist.size(); i++) {
		std::cout <<"x: " << plist[i].x << ", y: " << plist[i].y << std::endl;
	}
	//find the interval length
	float intervalLen = 0.0;
	for(size_t i = 1; i < plist.size(); i++) {
		intervalLen += glm::distance(plist[i-1], plist[i]);
	}
	intervalLen = intervalLen/(RESAMPLE_POINT_COUNT - 1);

	resampledPlist[0] = plist[0];
	
	size_t pointsAdded = 1;
	float currInterval = 0.0;
	
	for(size_t i = 1; i < plist.size(); i++) {
		float intervalStep = glm::distance(plist[i-1], plist[i]);
		if (currInterval + intervalStep >= intervalLen && pointsAdded < RESAMPLE_POINT_COUNT) {
			Vec2f newPoint;
			newPoint.x = plist[i-1].x + ((intervalLen - currInterval)/intervalStep)*(plist[i].x - plist[i-1].x);
			newPoint.y = plist[i-1].y + ((intervalLen - currInterval)/intervalStep)*(plist[i].y - plist[i-1].y);
		resampledPlist[pointsAdded++] = newPoint;
		plist.insert(plist.begin() + i, newPoint);
		currInterval = 0.0;
		} else {
			currInterval += intervalStep;
		}
	}
	if (pointsAdded == RESAMPLE_POINT_COUNT - 1) {
		resampledPlist[pointsAdded] = plist.back();
	}
}

/*!
 * Convert a vector of input points to a sequence of directions
 */
void RuneRecognitionAlt::vectorizeInput() {
	std::cout << "RESAMPED LIST: " << std::endl;
	for (size_t i = 0; i < RESAMPLE_POINT_COUNT; i++) {
		std::cout << "x: " << resampledPlist[i].x << ", y: " << resampledPlist[i].y << std::endl;
	}
	Vec2f centroid;
	for (size_t i = 0; i < RESAMPLE_POINT_COUNT; i++) {
		centroid.x += resampledPlist[i].x;
		centroid.y += resampledPlist[i].y;
	}
	centroid.x = centroid.x/RESAMPLE_POINT_COUNT;
	centroid.y = centroid.y/RESAMPLE_POINT_COUNT;
	for (size_t i = 0; i < RESAMPLE_POINT_COUNT; i++) {
		resampledPlist[i].x -= centroid.x;
		resampledPlist[i].y -= centroid.y;
	}
	float indicativeAngle = atan2(resampledPlist[0].y, resampledPlist[0].x);
	float baseOrientation = (glm::pi<float>()/4.0f)*glm::floor((indicativeAngle+glm::pi<float>()/8.0f)/(glm::pi<float>()/4.0f));
	float delta = baseOrientation - indicativeAngle;
	float magnitude = 0;
	for (size_t i = 0; i < RESAMPLE_POINT_COUNT; i++) {
		float newX = resampledPlist[i].x*glm::cos(delta) - resampledPlist[i].y*glm::sin(delta);
		float newY = resampledPlist[i].y*glm::cos(delta) + resampledPlist[i].x*glm::sin(delta);
		VecPlist[i*2] = newX;
		VecPlist[i*2+1] = newY;
		magnitude += newX*newX + newY*newY;
	}
	magnitude = sqrtf(magnitude);
	for (size_t i = 0; i < RESAMPLE_POINT_COUNT*2; i++) {
		VecPlist[i] = VecPlist[i]/magnitude;
	}
}

float RuneRecognitionAlt::optimalCosDistance(const float* templ) {
	float dotProduct = 0.0f;
	float angNumer = 0.0f;
	for (size_t i = 0; i < RESAMPLE_POINT_COUNT; i++) {	
		dotProduct += VecPlist[i*2]*templ[i*2] + VecPlist[i*2+1]*templ[i*2+1];
		angNumer = templ[i*2]*VecPlist[i*2+1] - templ[i*2+1]*VecPlist[i*2];
	}
	float angle = atan(angNumer/dotProduct);
	return acos(dotProduct*glm::cos(angle)+angNumer*glm::sin(angle));
}

int RuneRecognitionAlt::findMatchingPattern(){
	for (size_t i = 0; i < RESAMPLE_POINT_COUNT*2; i++) {
		std::cout << VecPlist[i] << ", ";
	}
	int index = -1;
	float maxScore = 0;
	for(size_t rune = 0; rune < RUNE_PATTERN_COUNT; rune++) {
		float score = 1/optimalCosDistance(patternData[rune].templateVec);
		LogInfo << "Score: " << score;
		if (score > maxScore) {
			maxScore = score;
			index = rune;
		}
	}
	
	if(maxScore >= PATTERN_SIMILARITY_THRESHOLD) {
		return index;
	} else {
		return -1;
	}
}

void RuneRecognitionAlt::analyze() {
	if(plist.size() < 2) {
		plist.clear();
		return;
	}
	
	resampleInput();
	vectorizeInput();

	int index = findMatchingPattern();
	if(index >= 0) {
		callRuneHandlers(index);
	} else {
		return;
	}
	
	bPrecastSpell = false;
	 
	// wanna precast?
	if(GInput->actionPressed(CONTROLS_CUST_STEALTHMODE)) {
		bPrecastSpell = true;
	}
}

RuneRecognitionAlt g_altRecognizer;

void ARX_SPELLS_Analyse_Alt() {
	g_altRecognizer.analyze();
}

static const size_t MAX_POINTS(200);

Rune SpellSymbol[MAX_SPELL_SYMBOLS];

size_t CurrSpellSymbol = 0;
std::string SpellMoves;

std::string LAST_FAILED_SEQUENCE = "none";

struct SpellDefinition {
	SpellDefinition * next[RUNE_COUNT];
	SpellType spell;
	SpellDefinition() : spell(SPELL_NONE) {
		for(size_t i = 0; i < RUNE_COUNT; i++) {
			next[i] = NULL;
		}
	}

	~SpellDefinition() {
		for(size_t i = 0; i < RUNE_COUNT; i++) {
			delete next[i];
		}
	}
};
static SpellDefinition definedSpells;

typedef std::map<std::string, SpellType> SpellNames;
static SpellNames spellNames;

struct RawSpellDefinition {
	SpellType spell;
	std::string name;
	Rune symbols[MAX_SPELL_SYMBOLS];
};

// TODO move to external file
static const RawSpellDefinition allSpells[] = {
	{SPELL_CURSE,                 "curse",                 {RUNE_RHAA,    RUNE_STREGUM,     RUNE_VITAE,   RUNE_NONE}}, // level 4
	{SPELL_FREEZE_TIME,           "freeze_time",           {RUNE_RHAA,    RUNE_TEMPUS,      RUNE_NONE}}, // level 10
	{SPELL_LOWER_ARMOR,           "lower_armor",           {RUNE_RHAA,    RUNE_KAOM,        RUNE_NONE}}, // level 2
	{SPELL_SLOW_DOWN,             "slowdown",              {RUNE_RHAA,    RUNE_MOVIS,       RUNE_NONE}}, // level 6
	{SPELL_HARM,                  "harm",                  {RUNE_RHAA,    RUNE_VITAE,       RUNE_NONE}}, // level 2
	{SPELL_CONFUSE,               "confuse",               {RUNE_RHAA,    RUNE_VISTA,       RUNE_NONE}}, // level 7
	{SPELL_MASS_PARALYSE,         "mass_paralyse",         {RUNE_MEGA,    RUNE_NHI,         RUNE_MOVIS,   RUNE_NONE}}, // level 9
	{SPELL_ARMOR,                 "armor",                 {RUNE_MEGA,    RUNE_KAOM,        RUNE_NONE}}, // level 2
	{SPELL_MAGIC_SIGHT,           "magic_sight",           {RUNE_MEGA,    RUNE_VISTA,       RUNE_NONE}}, // level 1
	{SPELL_HEAL,                  "heal",                  {RUNE_MEGA,    RUNE_VITAE,       RUNE_NONE}}, // level 2
	{SPELL_SPEED,                 "speed",                 {RUNE_MEGA,    RUNE_MOVIS,       RUNE_NONE}}, // level 3
	{SPELL_BLESS,                 "bless",                 {RUNE_MEGA,    RUNE_STREGUM,     RUNE_VITAE,   RUNE_NONE}}, // level 4
	{SPELL_ENCHANT_WEAPON,        "enchant_weapon",        {RUNE_MEGA,    RUNE_STREGUM,     RUNE_COSUM,   RUNE_NONE}}, // level 8
	{SPELL_MASS_INCINERATE,       "mass_incinerate",       {RUNE_MEGA,    RUNE_AAM,         RUNE_MEGA,    RUNE_YOK, RUNE_NONE}}, // level 10
	{SPELL_ACTIVATE_PORTAL,       "activate_portal",       {RUNE_MEGA,    RUNE_SPACIUM,     RUNE_NONE}}, // level ?
	{SPELL_LEVITATE,              "levitate",              {RUNE_MEGA,    RUNE_SPACIUM,     RUNE_MOVIS,   RUNE_NONE}}, // level 5
	{SPELL_PARALYSE,              "paralyse",              {RUNE_NHI,     RUNE_MOVIS,       RUNE_NONE}}, // level 6
	{SPELL_CURE_POISON,           "cure_poison",           {RUNE_NHI,     RUNE_CETRIUS,     RUNE_NONE}}, // level 5
	{SPELL_DOUSE,                 "douse",                 {RUNE_NHI,     RUNE_YOK,         RUNE_NONE}}, // level 1
	{SPELL_DISPELL_ILLUSION,      "dispell_illusion",      {RUNE_NHI,     RUNE_STREGUM,     RUNE_VISTA,   RUNE_NONE}}, // level 3
	{SPELL_NEGATE_MAGIC,          "negate_magic",          {RUNE_NHI,     RUNE_STREGUM,     RUNE_SPACIUM, RUNE_NONE}}, // level 9
	{SPELL_DISPELL_FIELD,         "dispell_field",         {RUNE_NHI,     RUNE_SPACIUM,     RUNE_NONE}}, // level 4
	{SPELL_DISARM_TRAP,           "disarm_trap",           {RUNE_NHI,     RUNE_MORTE,       RUNE_COSUM,   RUNE_NONE}}, // level 6
	{SPELL_INVISIBILITY,          "invisibility",          {RUNE_NHI,     RUNE_VISTA,       RUNE_NONE}}, // level ?
	{SPELL_FLYING_EYE,            "flying_eye",            {RUNE_VISTA,   RUNE_MOVIS,       RUNE_NONE}}, // level 7
	{SPELL_REPEL_UNDEAD,          "repel_undead",          {RUNE_MORTE,   RUNE_KAOM,        RUNE_NONE}}, // level 5
	{SPELL_DETECT_TRAP,           "detect_trap",           {RUNE_MORTE,   RUNE_COSUM,       RUNE_VISTA,   RUNE_NONE}}, // level 2
	{SPELL_CONTROL_TARGET,        "control",               {RUNE_MOVIS,   RUNE_COMUNICATUM, RUNE_NONE}}, // level 10
	{SPELL_MANA_DRAIN,            "mana_drain",            {RUNE_STREGUM, RUNE_MOVIS,       RUNE_NONE}}, // level 8
	{SPELL_INCINERATE,            "incinerate",            {RUNE_AAM,     RUNE_MEGA,        RUNE_YOK,     RUNE_NONE}}, // level 9
	{SPELL_EXPLOSION,             "explosion",             {RUNE_AAM,     RUNE_MEGA,        RUNE_MORTE,   RUNE_NONE}}, // level 8
	{SPELL_CREATE_FIELD,          "create_field",          {RUNE_AAM,     RUNE_KAOM,        RUNE_SPACIUM, RUNE_NONE}}, // level 6
	{SPELL_RISE_DEAD,             "raise_dead",            {RUNE_AAM,     RUNE_MORTE,       RUNE_VITAE,   RUNE_NONE}}, // level 6
	{SPELL_RUNE_OF_GUARDING,      "rune_of_guarding",      {RUNE_AAM,     RUNE_MORTE,       RUNE_COSUM,   RUNE_NONE}}, // level 5
	{SPELL_SUMMON_CREATURE,       "summon_creature",       {RUNE_AAM,     RUNE_VITAE,       RUNE_TERA,    RUNE_NONE}}, // level 9
	{SPELL_CREATE_FOOD,           "create_food",           {RUNE_AAM,     RUNE_VITAE,       RUNE_COSUM,   RUNE_NONE}}, // level 3
	{SPELL_LIGHTNING_STRIKE,      "lightning_strike",      {RUNE_AAM,     RUNE_FOLGORA,     RUNE_TAAR,    RUNE_NONE}}, // level 7
	{SPELL_MASS_LIGHTNING_STRIKE, "mass_lightning_strike", {RUNE_AAM,     RUNE_FOLGORA,     RUNE_SPACIUM, RUNE_NONE}}, // level 10
	{SPELL_IGNIT,                 "ignit",                 {RUNE_AAM,     RUNE_YOK,         RUNE_NONE}}, // level 1
	{SPELL_FIRE_FIELD,            "fire_field",            {RUNE_AAM,     RUNE_YOK,         RUNE_SPACIUM, RUNE_NONE}}, // level 7
	{SPELL_FIREBALL,              "fireball",              {RUNE_AAM,     RUNE_YOK,         RUNE_TAAR,    RUNE_NONE}}, // level 3
	{SPELL_ICE_FIELD,             "ice_field",             {RUNE_AAM,     RUNE_FRIDD,       RUNE_SPACIUM, RUNE_NONE}}, // level 7
	{SPELL_ICE_PROJECTILE,        "ice_projectile",        {RUNE_AAM,     RUNE_FRIDD,       RUNE_TAAR,    RUNE_NONE}}, // level 3
	{SPELL_POISON_PROJECTILE,     "poison_projectile",     {RUNE_AAM,     RUNE_CETRIUS,     RUNE_TAAR,    RUNE_NONE}}, // level 5
	{SPELL_MAGIC_MISSILE,         "magic_missile",         {RUNE_AAM,     RUNE_TAAR,        RUNE_NONE}}, // level 1
	{SPELL_FIRE_PROTECTION,       "fire_protection",       {RUNE_YOK,     RUNE_KAOM,        RUNE_NONE}}, // level 4
	{SPELL_COLD_PROTECTION,       "cold_protection",       {RUNE_FRIDD,   RUNE_KAOM,        RUNE_NONE}}, // level 4
	{SPELL_LIFE_DRAIN,            "life_drain",            {RUNE_VITAE,   RUNE_MOVIS,       RUNE_NONE}}, // level 8
	{SPELL_TELEKINESIS,           "telekinesis",           {RUNE_SPACIUM, RUNE_COMUNICATUM, RUNE_NONE}}, // level 4
	{SPELL_FAKE_SUMMON,           "fake_summon",           {RUNE_NONE}}
};

static void addSpell(const Rune symbols[MAX_SPELL_SYMBOLS], SpellType spell, const std::string & name) {
	
	typedef std::pair<SpellNames::const_iterator, bool> Res;
	Res res = spellNames.insert(std::make_pair(name, spell));
	if(!res.second) {
		LogWarning << "Duplicate spell name: " + name;
	}
	
	if(symbols[0] == RUNE_NONE) {
		return;
	}
	
	SpellDefinition * def = &definedSpells;
	
	for(size_t i = 0; i < MAX_SPELL_SYMBOLS; i++) {
		if(symbols[i] == RUNE_NONE) {
			break;
		}
		arx_assert(symbols[i] >= 0 && (size_t)symbols[i] < RUNE_COUNT);
		if(def->next[symbols[i]] == NULL) {
			def->next[symbols[i]] = new SpellDefinition();
		}
		def = def->next[symbols[i]];
	}
	
	arx_assert(def->spell == SPELL_NONE);
	
	def->spell = spell;
}

void spellRecognitionInit() {
	
	for(size_t i = 0; i < ARRAY_SIZE(allSpells); i++) {
		addSpell(allSpells[i].symbols, allSpells[i].spell, allSpells[i].name);
	}

	plist.reserve(MAX_POINTS);
}

//-----------------------------------------------------------------------------
// Resets Spell Recognition
void ARX_SPELLS_ResetRecognition() {
	
	for(size_t i = 0; i < MAX_SPELL_SYMBOLS; i++) {
		SpellSymbol[i] = RUNE_NONE;
	}
	
	for(size_t i = 0; i < 6; i++) {
		player.SpellToMemorize.iSpellSymbols[i] = RUNE_NONE;
	}
	
	CurrSpellSymbol = 0;
}


void spellRecognitionPointsReset() {
	plist.clear();
}

// Adds a 2D point to currently drawn spell symbol
void ARX_SPELLS_AddPoint(const Vec2s & pos) {
	
	if(plist.size() && Vec2f(pos) == plist.back()) {
		return;
	}

	if(plist.size() == MAX_POINTS) {
		plist.pop_back();
	}

	plist.push_back(Vec2f(pos));
}




SpellType GetSpellId(const std::string & spell) {
	
	SpellNames::const_iterator it = spellNames.find(spell);
	
	return (it == spellNames.end()) ? SPELL_NONE : it->second;
}

enum ARX_SPELLS_RuneDirection
{
	AUP,
	AUPRIGHT,
	ARIGHT,
	ADOWNRIGHT,
	ADOWN,
	ADOWNLEFT,
	ALEFT,
	AUPLEFT
};





static SpellType getSpell(const Rune symbols[MAX_SPELL_SYMBOLS]) {
	
	const SpellDefinition * def = &definedSpells;
	
	for(size_t i = 0; i < MAX_SPELL_SYMBOLS; i++) {
		if(symbols[i] == RUNE_NONE) {
			break;
		}
		arx_assert(symbols[i] >= 0 && (size_t)symbols[i] < RUNE_COUNT);
		if(def->next[symbols[i]] == NULL) {
			return SPELL_NONE;
		}
		def = def->next[symbols[i]];
	}
	
	return def->spell;
}



void ARX_SPELLS_Analyse() {
	
	unsigned char dirs[MAX_POINTS];
	unsigned char lastdir = 255;
	long cdir = 0;

	for(size_t i = 1; i < plist.size() ; i++) {
		
		Vec2f d = Vec2f(plist[i-1] - plist[i]);
		
		if(arx::length2(d) > 100) {
			
			float a = std::abs(d.x);
			float b = std::abs(d.y);
			
			if(b != 0.f && a / b > 0.4f && a / b < 2.5f) {
				// Diagonal movemement.
				
				if(d.x < 0 && d.y < 0) {
					if(lastdir != ADOWNRIGHT) {
						lastdir = dirs[cdir++] = ADOWNRIGHT;
					}
				} else if(d.x > 0 && d.y < 0) {
					if(lastdir != ADOWNLEFT) {
						lastdir = dirs[cdir++] = ADOWNLEFT;
					}
				} else if(d.x < 0 && d.y > 0) {
					if(lastdir != AUPRIGHT) {
						lastdir = dirs[cdir++] = AUPRIGHT;
					}
				} else if(d.x > 0 && d.y > 0) {
					if(lastdir != AUPLEFT) {
						lastdir = dirs[cdir++] = AUPLEFT;
					}
				}
				
			} else if(a > b) {
				// Horizontal movement.
				
				if(d.x < 0) {
					if(lastdir != ARIGHT) {
						lastdir = dirs[cdir++] = ARIGHT;
					}
				} else {
					if(lastdir != ALEFT) {
						lastdir = dirs[cdir++] = ALEFT;
					}
				}
				
			} else {
				// Vertical movement.
				
				if(d.y < 0) {
					if(lastdir != ADOWN) {
						lastdir = dirs[cdir++] = ADOWN;
					}
				} else {
					if(lastdir != AUP) {
						lastdir = dirs[cdir++] = AUP;
					}
				}
			}
		}
	}

	SpellMoves.clear();
	
	if ( cdir > 0 )
	{

		for (long i = 0 ; i < cdir ; i++ )
		{
			switch ( dirs[i] )
			{
				case AUP:
					SpellMoves += "8"; //uses PAD values
					break;

				case ADOWN:
					SpellMoves += "2";
					break;

				case ALEFT:
					SpellMoves += "4";
					break;

				case ARIGHT:
					SpellMoves += "6";
					break;

				case AUPRIGHT:
					SpellMoves += "9";
					break;

				case ADOWNRIGHT:
					SpellMoves += "3";
					break;

				case AUPLEFT:
					SpellMoves += "7";
					break;

				case ADOWNLEFT:
					SpellMoves += "1";
					break;
			}
		}
	}
}

static void handleRuneDetection(Rune rune) {
	SpellSymbol[CurrSpellSymbol++] = rune;

	if(CurrSpellSymbol >= MAX_SPELL_SYMBOLS) {
		CurrSpellSymbol = MAX_SPELL_SYMBOLS - 1;
	}

	ARX_SOUND_PlaySFX(SND_SYMB[rune]);
}

void ARX_SPELLS_AnalyseSYMBOL() {
	
	long sm = 0;
	try {
		sm = boost::lexical_cast<long>(SpellMoves);
	} catch(...) {
		LogDebug("bad spell moves: " << SpellMoves);
	}
	
	switch(sm) {
		
		// COSUM
		case 62148  :
		case 632148 :
		case 62498  :
		case 62748  :
		case 6248   :
			handleRuneDetection(RUNE_COSUM);
			break;
		// COMUNICATUM
		case 632426 :
		case 627426 :
		case 634236 :
		case 624326 :
		case 62426  :
			handleCheatRuneDetection(CheatRune_COMUNICATUM);
			handleRuneDetection(RUNE_COMUNICATUM);
			break;
		// FOLGORA
		case 9823   :
		case 9232   :
		case 983    :
		case 963    :
		case 923    :
		case 932    :
		case 93     :
			handleRuneDetection(RUNE_FOLGORA);
			break;
		// SPACIUM
		case 42368  :
		case 42678  :
		case 42698  :
		case 4268   :
			handleCheatRuneDetection(CheatRune_SPACIUM);
			handleRuneDetection(RUNE_SPACIUM);
			break;
		// TERA
		case 9826   :
		case 92126  :
		case 9264   :
		case 9296   :
		case 926    :
			handleRuneDetection(RUNE_TERA);
			break;
		// CETRIUS
		case 286   :
		case 3286  :
		case 23836 :
		case 38636 :
		case 2986  :
		case 2386  :
		case 386   :
			handleRuneDetection(RUNE_CETRIUS);
			break;
		// RHAA
		case 28    :
		case 2     :
			handleRuneDetection(RUNE_RHAA);
			break;
		// FRIDD
		case 98362	:
		case 8362	:
		case 8632	:
		case 8962	:
		case 862	:
			handleRuneDetection(RUNE_FRIDD);
			break;
		// KAOM
		case 41236	:
		case 23		:
		case 236	:
		case 2369	:
		case 136	:
		case 12369	:
		case 1236	:
			handleCheatRuneDetection(CheatRune_KAOM);
			handleRuneDetection(RUNE_KAOM);
			break;
		// STREGUM
		case 82328 :
		case 8328  :
		case 2328  :
		case 8938  :
		case 8238  :
		case 838   :
			handleCheatRuneDetection(CheatRune_STREGUM);
			handleRuneDetection(RUNE_STREGUM);
			break;
		// MORTE
		case 628   :
		case 621   :
		case 62    :
			handleRuneDetection(RUNE_MORTE);
			break;
		// TEMPUS
		case 962686  :
		case 862686  :
		case 8626862 : 
			handleRuneDetection(RUNE_TEMPUS);
			break;
		// MOVIS
		case 6316:
		case 61236:
		case 6146:
		case 61216:
		case 6216:
		case 6416:
		case 62126:
		case 61264:
		case 6126:
		case 6136:
		case 616: 
			handleRuneDetection(RUNE_MOVIS);
			break;
		// NHI
		case 46:
		case 4:
			handleRuneDetection(RUNE_NHI);
			break;
		// AAM
		case 64:
		case 6:
			handleCheatRuneDetection(CheatRune_AAM);
			handleRuneDetection(RUNE_AAM);
			break;
		// YOK
		case 412369:
		case 2687:
		case 2698:
		case 2638:
		case 26386:
		case 2368:
		case 2689:
		case 268:
			handleRuneDetection(RUNE_YOK);
			break;
		// TAAR
		case 6236:
		case 6264:
		case 626:
			handleRuneDetection(RUNE_TAAR);
			break;
		// MEGA
		case 82:
		case 8:
			handleCheatRuneDetection(CheatRune_MEGA);
			handleRuneDetection(RUNE_MEGA);
			break;
		// VISTA
		case 3614:
		case 361:
		case 341:
		case 3212:
		case 3214:
		case 312:
		case 314:
		case 321:
		case 31:
			handleRuneDetection(RUNE_VISTA);
			break;
		// VITAE
		case 698:
		case 68:
			handleRuneDetection(RUNE_VITAE);
			break;
//-----------------------------------------------
// Cheat spells
		// Special UW mode
		case 238:
		case 2398:
		case 23898:
		case 236987:
		case 23698:
			handleCheatRuneDetection(CheatRune_U);
			goto failed; 
		case 2382398:
		case 2829:
		case 23982398:
		case 39892398:
		case 2398938:
		case 28239898:
		case 238982398:
		case 238923898:
		case 28982398:
		case 3923989:
		case 292398:
		case 398329:
		case 38923898:
		case 2398289:
		case 289823898:
		case 2989238:
		case 29829:
		case 2393239:
		case 38239:
		case 239829:
		case 2898239:
		case 28982898:
		case 389389:
		case 3892389:
		case 289289:
		case 289239:
		case 239289:
		case 2989298:
		case 2392398:
		case 238929:
		case 28923898:
		case 2929:
		case 2398298:
		case 239823898:
		case 28238:
		case 2892398:
		case 28298:
		case 298289:
		case 38929:
		case 289298989:
		case 23892398:
		case 238239:
		case 29298:
		case 2329298:
		case 232389829:
		case 2389829:
		case 239239:
		case 282398:
		case 2389239:
		case 2929898:
		case 3292398:
		case 23923298:
		case 23898239:
		case 3232929:
		case 2982398:
		case 238298:
		case 3939:
			handleCheatRuneDetection(CheatRune_W);
			goto failed; 
		case 161:
		case 1621:
		case 1261:
			handleCheatRuneDetection(CheatRune_S);
			goto failed;
		case 83614:
		case 8361:
		case 8341:
		case 83212:
		case 83214:
		case 8312:
		case 8314:
		case 8321:
		case 831:
		case 82341:
		case 834:
		case 823:
		case 8234:
		case 8231:
			handleCheatRuneDetection(CheatRune_P);
			goto failed;
		case 83692:
		case 823982:
		case 83982:
		case 82369892:
		case 82392:
		case 83892:
		case 823282:
		case 8392:
			handleCheatRuneDetection(CheatRune_M);
			goto failed;
		case 98324:
		case 92324:
		case 89324:
		case 9324:
		case 9892324:
		case 9234:
		case 934:
			handleCheatRuneDetection(CheatRune_A);
			goto failed;
		case 3249:
		case 2349:
		case 323489:
		case 23249:
		case 3489:
		case 32498:
		case 349:
			handleCheatRuneDetection(CheatRune_X);
			goto failed;
		case 26:
			handleCheatRuneDetection(CheatRune_26);
			goto failed;
		case 9232187:
		case 93187:
		case 9234187:
		case 831878:
		case 923187:
		case 932187:
		case 93217:
		case 9317:
			handleCheatRuneDetection(CheatRune_O);
			goto failed;
		case 82313:
		case 8343:
		case 82343:
		case 83413:
		case 8313:
			handleCheatRuneDetection(CheatRune_R);
			goto failed;
		case 86:
			handleCheatRuneDetection(CheatRune_F);
			goto failed;
		case 626262:
			handleCheatRuneDetection(CheatRune_Passwall);
			break;
		case 828282:
			handleCheatRuneDetection(CheatRune_ChangeSkin);
			goto failed;
		default: {
		failed:
			;
			std::string tex;

			if(SpellMoves.length()>=127)
				SpellMoves.resize(127);

			LAST_FAILED_SEQUENCE = SpellMoves;

			LogDebug("Unknown Symbol - " + SpellMoves);
		}
	}

	bPrecastSpell = false;

	// wanna precast?
	if(GInput->actionPressed(CONTROLS_CUST_STEALTHMODE)) {
		bPrecastSpell = true;
	}
}

bool ARX_SPELLS_AnalyseSPELL() {
	
	SpellcastFlags flags = 0;
	
	if(GInput->actionPressed(CONTROLS_CUST_STEALTHMODE) || bPrecastSpell) {
		flags |= SPELLCAST_FLAG_PRECAST;
	}
	
	bPrecastSpell = false;
	
	SpellType spell;
	
	if(SpellSymbol[0] == RUNE_MEGA && SpellSymbol[1] == RUNE_MEGA
	   && SpellSymbol[2] == RUNE_MEGA && SpellSymbol[3] == RUNE_AAM
	   && SpellSymbol[4] == RUNE_VITAE && SpellSymbol[5] == RUNE_TERA) {
		cur_mega = 10;
		spell = SPELL_SUMMON_CREATURE;
	} else {
		spell = getSpell(SpellSymbol);
	}
	
	if(spell == SPELL_NONE) {
		ARX_SOUND_PlaySFX(SND_MAGIC_FIZZLE);
		
		if(player.SpellToMemorize.bSpell) {
			CurrSpellSymbol = 0;
			player.SpellToMemorize.bSpell = false;
		}
		
		return false;
	}
	
	return ARX_SPELLS_Launch(spell,
	                         EntityHandle_Player,
	                         flags,
	                         -1,
	                         EntityHandle(),
	                         ArxDuration(-1));
	
}
