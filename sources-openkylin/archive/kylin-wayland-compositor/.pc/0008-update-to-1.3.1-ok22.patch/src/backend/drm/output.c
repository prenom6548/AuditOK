// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <stdlib.h>

#include <kywc/log.h>

#include "drm_p.h"
#include "output.h"
#include "util/quirks.h"

/**
 * From Redshift project:
 * https://github.com/jonls/redshift/blob/master/src/colorramp.c
 *
 * Copyright (c) 2013-2014  Jon Lund Steffensen <jonlst@gmail.com>
 * Copyright (c) 2013  Ingo Thies <ithies@astro.uni-bonn.de>
 */

// clang-format off
static const float blackbody_color[] = {
    1.00000000, 0.18172716, 0.00000000, /* 1000K */
    1.00000000, 0.25503671, 0.00000000, /* 1100K */
    1.00000000, 0.30942099, 0.00000000, /* 1200K */
    1.00000000, 0.35357379, 0.00000000, /* ...   */
    1.00000000, 0.39091524, 0.00000000,
    1.00000000, 0.42322816, 0.00000000,
    1.00000000, 0.45159884, 0.00000000,
    1.00000000, 0.47675916, 0.00000000,
    1.00000000, 0.49923747, 0.00000000,
    1.00000000, 0.51943421, 0.00000000,
    1.00000000, 0.54360078, 0.08679949,
    1.00000000, 0.56618736, 0.14065513,
    1.00000000, 0.58734976, 0.18362641,
    1.00000000, 0.60724493, 0.22137978,
    1.00000000, 0.62600248, 0.25591950,
    1.00000000, 0.64373109, 0.28819679,
    1.00000000, 0.66052319, 0.31873863,
    1.00000000, 0.67645822, 0.34786758,
    1.00000000, 0.69160518, 0.37579588,
    1.00000000, 0.70602449, 0.40267128,
    1.00000000, 0.71976951, 0.42860152,
    1.00000000, 0.73288760, 0.45366838,
    1.00000000, 0.74542112, 0.47793608,
    1.00000000, 0.75740814, 0.50145662,
    1.00000000, 0.76888303, 0.52427322,
    1.00000000, 0.77987699, 0.54642268,
    1.00000000, 0.79041843, 0.56793692,
    1.00000000, 0.80053332, 0.58884417,
    1.00000000, 0.81024551, 0.60916971,
    1.00000000, 0.81957693, 0.62893653,
    1.00000000, 0.82854786, 0.64816570,
    1.00000000, 0.83717703, 0.66687674,
    1.00000000, 0.84548188, 0.68508786,
    1.00000000, 0.85347859, 0.70281616,
    1.00000000, 0.86118227, 0.72007777,
    1.00000000, 0.86860704, 0.73688797,
    1.00000000, 0.87576611, 0.75326132,
    1.00000000, 0.88267187, 0.76921169,
    1.00000000, 0.88933596, 0.78475236,
    1.00000000, 0.89576933, 0.79989606,
    1.00000000, 0.90198230, 0.81465502,
    1.00000000, 0.90963069, 0.82838210,
    1.00000000, 0.91710889, 0.84190889,
    1.00000000, 0.92441842, 0.85523742,
    1.00000000, 0.93156127, 0.86836903,
    1.00000000, 0.93853986, 0.88130458,
    1.00000000, 0.94535695, 0.89404470,
    1.00000000, 0.95201559, 0.90658983,
    1.00000000, 0.95851906, 0.91894041,
    1.00000000, 0.96487079, 0.93109690,
    1.00000000, 0.97107439, 0.94305985,
    1.00000000, 0.97713351, 0.95482993,
    1.00000000, 0.98305189, 0.96640795,
    1.00000000, 0.98883326, 0.97779486,
    1.00000000, 0.99448139, 0.98899179,
    1.00000000, 1.00000000, 1.00000000, /* 6500K */
    0.98947904, 0.99348723, 1.00000000,
    0.97940448, 0.98722715, 1.00000000,
    0.96975025, 0.98120637, 1.00000000,
    0.96049223, 0.97541240, 1.00000000,
    0.95160805, 0.96983355, 1.00000000,
    0.94303638, 0.96443333, 1.00000000,
    0.93480451, 0.95923080, 1.00000000,
    0.92689056, 0.95421394, 1.00000000,
    0.91927697, 0.94937330, 1.00000000,
    0.91194747, 0.94470005, 1.00000000,
    0.90488690, 0.94018594, 1.00000000,
    0.89808115, 0.93582323, 1.00000000,
    0.89151710, 0.93160469, 1.00000000,
    0.88518247, 0.92752354, 1.00000000,
    0.87906581, 0.92357340, 1.00000000,
    0.87315640, 0.91974827, 1.00000000,
    0.86744421, 0.91604254, 1.00000000,
    0.86191983, 0.91245088, 1.00000000,
    0.85657444, 0.90896831, 1.00000000,
    0.85139976, 0.90559011, 1.00000000,
    0.84638799, 0.90231183, 1.00000000,
    0.84153180, 0.89912926, 1.00000000,
    0.83682430, 0.89603843, 1.00000000,
    0.83225897, 0.89303558, 1.00000000,
    0.82782969, 0.89011714, 1.00000000,
    0.82353066, 0.88727974, 1.00000000,
    0.81935641, 0.88452017, 1.00000000,
    0.81530175, 0.88183541, 1.00000000,
    0.81136180, 0.87922257, 1.00000000,
    0.80753191, 0.87667891, 1.00000000,
    0.80380769, 0.87420182, 1.00000000,
    0.80018497, 0.87178882, 1.00000000,
    0.79665980, 0.86943756, 1.00000000,
    0.79322843, 0.86714579, 1.00000000,
    0.78988728, 0.86491137, 1.00000000, /* 10000K */
    0.78663296, 0.86273225, 1.00000000,
    0.78346225, 0.86060650, 1.00000000,
    0.78037207, 0.85853224, 1.00000000,
    0.77735950, 0.85650771, 1.00000000,
    0.77442176, 0.85453121, 1.00000000,
    0.77155617, 0.85260112, 1.00000000,
    0.76876022, 0.85071588, 1.00000000,
    0.76603147, 0.84887402, 1.00000000,
    0.76336762, 0.84707411, 1.00000000,
    0.76076645, 0.84531479, 1.00000000,
    0.75822586, 0.84359476, 1.00000000,
    0.75574383, 0.84191277, 1.00000000,
    0.75331843, 0.84026762, 1.00000000,
    0.75094780, 0.83865816, 1.00000000,
    0.74863017, 0.83708329, 1.00000000,
    0.74636386, 0.83554194, 1.00000000,
    0.74414722, 0.83403311, 1.00000000,
    0.74197871, 0.83255582, 1.00000000,
    0.73985682, 0.83110912, 1.00000000,
    0.73778012, 0.82969211, 1.00000000,
    0.73574723, 0.82830393, 1.00000000,
    0.73375683, 0.82694373, 1.00000000,
    0.73180765, 0.82561071, 1.00000000,
    0.72989845, 0.82430410, 1.00000000,
    0.72802807, 0.82302316, 1.00000000,
    0.72619537, 0.82176715, 1.00000000,
    0.72439927, 0.82053539, 1.00000000,
    0.72263872, 0.81932722, 1.00000000,
    0.72091270, 0.81814197, 1.00000000,
    0.71922025, 0.81697905, 1.00000000,
    0.71756043, 0.81583783, 1.00000000,
    0.71593234, 0.81471775, 1.00000000,
    0.71433510, 0.81361825, 1.00000000,
    0.71276788, 0.81253878, 1.00000000,
    0.71122987, 0.81147883, 1.00000000,
    0.70972029, 0.81043789, 1.00000000,
    0.70823838, 0.80941546, 1.00000000,
    0.70678342, 0.80841109, 1.00000000,
    0.70535469, 0.80742432, 1.00000000,
    0.70395153, 0.80645469, 1.00000000,
    0.70257327, 0.80550180, 1.00000000,
    0.70121928, 0.80456522, 1.00000000,
    0.69988894, 0.80364455, 1.00000000,
    0.69858167, 0.80273941, 1.00000000,
    0.69729688, 0.80184943, 1.00000000,
    0.69603402, 0.80097423, 1.00000000,
    0.69479255, 0.80011347, 1.00000000,
    0.69357196, 0.79926681, 1.00000000,
    0.69237173, 0.79843391, 1.00000000,
    0.69119138, 0.79761446, 1.00000000, /* 15000K */
    0.69003044, 0.79680814, 1.00000000,
    0.68888844, 0.79601466, 1.00000000,
    0.68776494, 0.79523371, 1.00000000,
    0.68665951, 0.79446502, 1.00000000,
    0.68557173, 0.79370830, 1.00000000,
    0.68450119, 0.79296330, 1.00000000,
    0.68344751, 0.79222975, 1.00000000,
    0.68241029, 0.79150740, 1.00000000,
    0.68138918, 0.79079600, 1.00000000,
    0.68038380, 0.79009531, 1.00000000,
    0.67939381, 0.78940511, 1.00000000,
    0.67841888, 0.78872517, 1.00000000,
    0.67745866, 0.78805526, 1.00000000,
    0.67651284, 0.78739518, 1.00000000,
    0.67558112, 0.78674472, 1.00000000,
    0.67466317, 0.78610368, 1.00000000,
    0.67375872, 0.78547186, 1.00000000,
    0.67286748, 0.78484907, 1.00000000,
    0.67198916, 0.78423512, 1.00000000,
    0.67112350, 0.78362984, 1.00000000,
    0.67027024, 0.78303305, 1.00000000,
    0.66942911, 0.78244457, 1.00000000,
    0.66859988, 0.78186425, 1.00000000,
    0.66778228, 0.78129191, 1.00000000,
    0.66697610, 0.78072740, 1.00000000,
    0.66618110, 0.78017057, 1.00000000,
    0.66539706, 0.77962127, 1.00000000,
    0.66462376, 0.77907934, 1.00000000,
    0.66386098, 0.77854465, 1.00000000,
    0.66310852, 0.77801705, 1.00000000,
    0.66236618, 0.77749642, 1.00000000,
    0.66163375, 0.77698261, 1.00000000,
    0.66091106, 0.77647551, 1.00000000,
    0.66019791, 0.77597498, 1.00000000,
    0.65949412, 0.77548090, 1.00000000,
    0.65879952, 0.77499315, 1.00000000,
    0.65811392, 0.77451161, 1.00000000,
    0.65743716, 0.77403618, 1.00000000,
    0.65676908, 0.77356673, 1.00000000,
    0.65610952, 0.77310316, 1.00000000,
    0.65545831, 0.77264537, 1.00000000,
    0.65481530, 0.77219324, 1.00000000,
    0.65418036, 0.77174669, 1.00000000,
    0.65355332, 0.77130560, 1.00000000,
    0.65293404, 0.77086988, 1.00000000,
    0.65232240, 0.77043944, 1.00000000,
    0.65171824, 0.77001419, 1.00000000,
    0.65112144, 0.76959404, 1.00000000,
    0.65053187, 0.76917889, 1.00000000,
    0.64994941, 0.76876866, 1.00000000, /* 20000K */
    0.64937392, 0.76836326, 1.00000000,
    0.64880528, 0.76796263, 1.00000000,
    0.64824339, 0.76756666, 1.00000000,
    0.64768812, 0.76717529, 1.00000000,
    0.64713935, 0.76678844, 1.00000000,
    0.64659699, 0.76640603, 1.00000000,
    0.64606092, 0.76602798, 1.00000000,
    0.64553103, 0.76565424, 1.00000000,
    0.64500722, 0.76528472, 1.00000000,
    0.64448939, 0.76491935, 1.00000000,
    0.64397745, 0.76455808, 1.00000000,
    0.64347129, 0.76420082, 1.00000000,
    0.64297081, 0.76384753, 1.00000000,
    0.64247594, 0.76349813, 1.00000000,
    0.64198657, 0.76315256, 1.00000000,
    0.64150261, 0.76281076, 1.00000000,
    0.64102399, 0.76247267, 1.00000000,
    0.64055061, 0.76213824, 1.00000000,
    0.64008239, 0.76180740, 1.00000000,
    0.63961926, 0.76148010, 1.00000000,
    0.63916112, 0.76115628, 1.00000000,
    0.63870790, 0.76083590, 1.00000000,
    0.63825953, 0.76051890, 1.00000000,
    0.63781592, 0.76020522, 1.00000000,
    0.63737701, 0.75989482, 1.00000000,
    0.63694273, 0.75958764, 1.00000000,
    0.63651299, 0.75928365, 1.00000000,
    0.63608774, 0.75898278, 1.00000000,
    0.63566691, 0.75868499, 1.00000000,
    0.63525042, 0.75839025, 1.00000000,
    0.63483822, 0.75809849, 1.00000000,
    0.63443023, 0.75780969, 1.00000000,
    0.63402641, 0.75752379, 1.00000000,
    0.63362667, 0.75724075, 1.00000000,
    0.63323097, 0.75696053, 1.00000000,
    0.63283925, 0.75668310, 1.00000000,
    0.63245144, 0.75640840, 1.00000000,
    0.63206749, 0.75613641, 1.00000000,
    0.63168735, 0.75586707, 1.00000000,
    0.63131096, 0.75560036, 1.00000000,
    0.63093826, 0.75533624, 1.00000000,
    0.63056920, 0.75507467, 1.00000000,
    0.63020374, 0.75481562, 1.00000000,
    0.62984181, 0.75455904, 1.00000000,
    0.62948337, 0.75430491, 1.00000000,
    0.62912838, 0.75405319, 1.00000000,
    0.62877678, 0.75380385, 1.00000000,
    0.62842852, 0.75355685, 1.00000000,
    0.62808356, 0.75331217, 1.00000000,
    0.62774186, 0.75306977, 1.00000000, /* 25000K */
    0.62740336, 0.75282962, 1.00000000  /* 25100K */
};

static float default_mat[] = {
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
};

static float grayscale_mat[] = {
    0.299f, 0.299f, 0.299f, 0.0f,
    0.587f, 0.587f, 0.587f, 0.0f,
    0.114f, 0.114f, 0.114f, 0.0f,
      0.0f,   0.0f,   0.0f, 1.0f
};

static float invert_mat[] = {
    -1.0f,  0.0f,  0.0f, 0.0f,
     0.0f, -1.0f,  0.0f, 0.0f,
     0.0f,  0.0f, -1.0f, 0.0f,
     1.0f,  1.0f,  1.0f, 1.0f
};

static float protanopia_mat[] = {
    1.0f, 0.5836f,  0.6384f, 0.0f,
    0.0f, 0.4146f, -0.6384f, 0.0f,
    0.0f,    0.0f,     1.0f, 0.0f,
    0.0f,    0.0f,     0.0f, 1.0f
};

static float deuteranopia_mat[] = {
     1.4027f, 0.1740f, -0.2818f, 0.0f,
    -0.8266f, 0.6429f,  0.5784f, 0.0f,
     0.4240f, 0.1832f,  0.7032f, 0.0f,
        0.0f,    0.0f,     0.0f, 1.0f
};

static float tritanopia_mat[] = {
          1.0f,       0.0f,      0.0f, 0.0f,
    -0.805712f,  0.378838f, 0.104823f, 0.0f,
     0.805712f,  0.621162f, 0.895177f, 0.0f,
          0.0f,       0.0f,      0.0f, 1.0f
};
// clang-format on

/* Helper macro used in the fill functions */
#define F(Y, C) pow((Y) * _brightness * whitepoint[C], 1.0)

static void interpolate_clolor(float a, const float *c1, const float *c2, float *c)
{
    c[0] = (1.0 - a) * c1[0] + a * c2[0];
    c[1] = (1.0 - a) * c1[1] + a * c2[1];
    c[2] = (1.0 - a) * c1[2] + a * c2[2];
}

void drm_color_temp_to_rgb(uint32_t color_temp, float *rgb)
{
    float alpha = (color_temp % 100) / 100.;
    int bbc_index = ((color_temp - 1000) / 100) * 3;
    interpolate_clolor(alpha, &blackbody_color[bbc_index], &blackbody_color[bbc_index + 3], rgb);
}

static void fill_gamma_ramp_with_color_temp(struct drm_color_lut *gamma_lut, uint32_t gamma_size,
                                            uint32_t color_temp, uint32_t brightness, bool invert)
{
    /* linear default state or invert state */
    for (uint32_t i = 0; i < gamma_size; i++) {
        uint16_t value = (double)i / gamma_size * (UINT16_MAX + 1);
        value = invert ? UINT16_MAX - value : value;
        gamma_lut[i].red = value;
        gamma_lut[i].green = value;
        gamma_lut[i].blue = value;
    }
    /* approximate white point */
    float whitepoint[3];
    drm_color_temp_to_rgb(color_temp, whitepoint);
    float _brightness = (float)brightness / 100;

    /* Helper macro used in the fill functions */
    for (uint32_t i = 0; i < gamma_size; i++) {
        gamma_lut[i].red = F((double)gamma_lut[i].red / (UINT16_MAX + 1), 0) * (UINT16_MAX + 1);
        gamma_lut[i].green = F((double)gamma_lut[i].green / (UINT16_MAX + 1), 1) * (UINT16_MAX + 1);
        gamma_lut[i].blue = F((double)gamma_lut[i].blue / (UINT16_MAX + 1), 2) * (UINT16_MAX + 1);
    }
}

static bool output_state_set_gamma_lut(struct drm_output_state *state,
                                       struct drm_color_lut *gamma_lut, size_t gamma_size)
{
    free(state->lut);

    state->lut = gamma_lut;
    state->lut_size = gamma_size;
    state->committed |= DRM_OUTPUT_STATE_GAMMA_LUT;

    return true;
}

static void output_set_brightness_state(struct output *output)
{
    struct drm_connector *conn = drm_connector_from_output(output->wlr_output);
    if (!conn || !conn->props[DRM_CONNECTOR_PROP_BRIGHTNESS].id) {
        return;
    }

    conn->state.brightness = output->base.state.brightness;
    conn->state.committed |= DRM_OUTPUT_STATE_BRIGHTNESS;
}

static void output_set_gamma_lut_state(struct output *output)
{
    if (output->quirks & QUIRKS_MASK_DISABLE_GAMMA) {
        return;
    }

    struct drm_connector *conn = drm_connector_from_output(output->wlr_output);
    if (!conn || !conn->crtc) {
        return;
    }

    size_t gamma_size = drm_get_crtc_gamma_lut_size(conn->crtc);
    if (gamma_size <= 1) {
        return;
    }

    uint32_t color_temp = output->base.state.color_temp;
    uint32_t brightness =
        conn->props[DRM_CONNECTOR_PROP_BRIGHTNESS].id ? 100 : output->base.state.brightness;

    struct drm_color_lut *gamma_lut = malloc(gamma_size * sizeof(struct drm_color_lut));
    fill_gamma_ramp_with_color_temp(gamma_lut, gamma_size, color_temp, brightness, false);
    output_state_set_gamma_lut(&conn->state, gamma_lut, gamma_size);
}

static void output_set_color_ctm_state(struct output *output)
{
    if (output->quirks & QUIRKS_MASK_DISABLE_CTM) {
        return;
    }

    struct drm_connector *conn = drm_connector_from_output(output->wlr_output);
    if (!conn || !conn->crtc || conn->crtc->props[DRM_CRTC_PROP_CTM].id == 0) {
        return;
    }

    float color_ctm[9] = { 1, 0, 0, 0, 1, 0, 0, 0, 1 };
    if (output->base.state.color_filter != KYWC_OUTPUT_COLOR_FILTER_INVERT) {
        const float *mat = drm_output_get_color_filter_matrix(output->base.state.color_filter);
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                color_ctm[i * 3 + j] = mat[j * 4 + i];
            }
        }
    }

    for (int i = 0; i < 9; i++) {
        // ctm values are in S31.32 sign-magnitude format
        uint64_t value = fabs(color_ctm[i]) * (1ull << 32);
        if (color_ctm[i] < 0) {
            value |= 1ull << 63;
        }
        conn->state.rgb_ctm.matrix[i] = value;
    }

    conn->state.committed |= DRM_OUTPUT_STATE_RGB_CTM;
}

static bool drm_output_is_force_soft_color(void)
{
    char *env = getenv("KYWC_SOFTWARE_COLOR");
    if (env && strcmp(env, "1") == 0) {
        return true;
    }

    return false;
}

void drm_output_set_state(struct output *output)
{
    struct drm_connector *conn = drm_connector_from_output(output->wlr_output);
    if (!conn->state.color_changed) {
        return;
    }
    conn->state.color_changed = false;

    if (drm_output_is_force_soft_color()) {
        return;
    }

    output_set_brightness_state(output);
    output_set_gamma_lut_state(output);
    output_set_color_ctm_state(output);
}

void wlr_output_set_buffer_damage(struct wlr_output *wlr_output, const pixman_region32_t *damage)
{
    if (!wlr_output_is_drm(wlr_output)) {
        return;
    }

    struct drm_connector *conn = drm_connector_from_output(wlr_output);
    if (!conn) {
        return;
    }

    pixman_region32_copy(&conn->state.damage, damage);
    conn->state.committed |= DRM_OUTPUT_STATE_DAMAGE;
    return;
}

void wlr_output_set_color_changed(struct wlr_output *wlr_output, bool changed)
{
    if (!wlr_output_is_drm(wlr_output)) {
        return;
    }

    struct drm_connector *conn = drm_connector_from_output(wlr_output);
    if (!conn) {
        return;
    }

    conn->state.color_changed = changed;
}

void wlr_output_set_rgb_range(struct wlr_output *wlr_output, uint32_t rgb_range)
{
    if (!wlr_output_is_drm(wlr_output)) {
        return;
    }

    struct drm_connector *conn = drm_connector_from_output(wlr_output);
    if (!conn) {
        return;
    }

    if (!conn->props[DRM_CONNECTOR_PROP_RGB_RANGE].id) {
        return;
    }

    if (rgb_range > KYWC_OUTPUT_RGB_RANGE_LIMITED) {
        return;
    }

    conn->state.committed |= DRM_OUTPUT_STATE_RGB_RANGE;
    conn->state.rgb_range = rgb_range;
}

void wlr_output_set_overscan(struct wlr_output *wlr_output, uint32_t overscan)
{
    if (!wlr_output_is_drm(wlr_output)) {
        return;
    }

    struct drm_connector *conn = drm_connector_from_output(wlr_output);
    if (!conn) {
        return;
    }

    if (!conn->props[DRM_CONNECTOR_PROP_OVERSCAN].id) {
        return;
    }

    conn->state.committed |= DRM_OUTPUT_STATE_OVERSCAN;
    conn->state.overscan = overscan;
}

void wlr_output_set_scaling_mode(struct wlr_output *wlr_output, uint32_t scaling_mode)
{
    if (!wlr_output_is_drm(wlr_output)) {
        return;
    }

    struct drm_connector *conn = drm_connector_from_output(wlr_output);
    if (!conn) {
        return;
    }

    if (!conn->props[DRM_CONNECTOR_PROP_SCALING_MODE].id) {
        return;
    }

    if ((scaling_mode > KYWC_OUTPUT_SCALING_MODE_FULL_ASPECT) ||
        (!conn->has_none_mode && scaling_mode == KYWC_OUTPUT_SCALING_MODE_NONE)) {

        return;
    }

    conn->state.committed |= DRM_OUTPUT_STATE_SCALING_MODE;
    conn->state.scaling_mode = scaling_mode;
}

void drm_output_state_clear(struct drm_output_state *state)
{
    free(state->lut);
    state->lut = NULL;
    state->committed = 0;
}

const void *wlr_output_get_edid(struct wlr_output *wlr_output, size_t *length)
{
    if (!wlr_output_is_drm(wlr_output)) {
        return NULL;
    }

    struct drm_connector *conn = drm_connector_from_output(wlr_output);
    if (!conn) {
        return NULL;
    }

    *length = conn->edid_len;
    return conn->edid;
}

bool wlr_output_get_vrr_capable(struct wlr_output *wlr_output)
{
    if (!wlr_output_is_drm(wlr_output)) {
        return false;
    }

    struct drm_connector *conn = drm_connector_from_output(wlr_output);
    return conn && conn->adaptive_sync_supported;
}

bool wlr_output_get_rgb_range(struct wlr_output *wlr_output, uint32_t *rgb_range)
{
    if (!wlr_output_is_drm(wlr_output)) {
        return false;
    }

    struct drm_connector *conn = drm_connector_from_output(wlr_output);
    if (!conn) {
        return false;
    }

    struct drm_prop_info *prop = &conn->props[DRM_CONNECTOR_PROP_RGB_RANGE];
    if (!prop->id) {
        return false;
    }

    *rgb_range = conn->rgb_range;
    return true;
}

bool wlr_output_get_overscan(struct wlr_output *wlr_output, uint32_t *overscan)
{
    if (!wlr_output_is_drm(wlr_output)) {
        return false;
    }

    struct drm_connector *conn = drm_connector_from_output(wlr_output);
    if (!conn) {
        return false;
    }

    struct drm_prop_info *prop = &conn->props[DRM_CONNECTOR_PROP_OVERSCAN];
    if (!prop->id) {
        return false;
    }

    *overscan = conn->overscan;
    return false;
}

bool wlr_output_get_scaling_mode(struct wlr_output *wlr_output, uint32_t *scaling_mode)
{
    if (!wlr_output_is_drm(wlr_output)) {
        return false;
    }

    struct drm_connector *conn = drm_connector_from_output(wlr_output);
    if (!conn) {
        return false;
    }

    struct drm_prop_info *prop = &conn->props[DRM_CONNECTOR_PROP_SCALING_MODE];
    if (!prop->id) {
        return false;
    }

    *scaling_mode = conn->scaling_mode;
    return true;
}

const float *drm_output_get_color_filter_matrix(enum kywc_output_color_filter color_filter)
{
    switch (color_filter) {
    case KYWC_OUTPUT_COLOR_FILTER_PROTANOPIA:
        return protanopia_mat;
    case KYWC_OUTPUT_COLOR_FILTER_DEUTERANOPIA:
        return deuteranopia_mat;
    case KYWC_OUTPUT_COLOR_FILTER_TRITANOPIA:
        return tritanopia_mat;
    case KYWC_OUTPUT_COLOR_FILTER_GRAYSCALE:
        return grayscale_mat;
    case KYWC_OUTPUT_COLOR_FILTER_INVERT:
        return invert_mat;
    default:
        return default_mat;
    }
}

bool drm_output_use_hw_brightness(struct wlr_output *wlr_output,
                                  const struct kywc_output_state *state)
{
    if (drm_output_is_force_soft_color()) {
        return state->brightness == 100;
    }

    struct drm_connector *conn = drm_connector_from_output(wlr_output);
    if (!conn || !conn->crtc) {
        return false;
    }

    if (state->color_feature == KYWC_OUTPUT_COLOR_FEATURE_DISABLE) {
        return true;
    }

    if (state->color_feature == KYWC_OUTPUT_COLOR_FEATURE_SOFTCOLOR) {
        return false;
    }

    if (state->color_feature == KYWC_OUTPUT_COLOR_FEATURE_SOFTGAMMA) {
        return conn->props[DRM_CONNECTOR_PROP_BRIGHTNESS].id;
    }

    struct output *output = output_from_wlr_output(wlr_output);
    if (output->quirks & QUIRKS_MASK_DISABLE_GAMMA) {
        return conn->props[DRM_CONNECTOR_PROP_BRIGHTNESS].id;
    }

    uint32_t gamma_size = drm_get_crtc_gamma_lut_size(conn->crtc);

    return state->brightness == 100 || conn->props[DRM_CONNECTOR_PROP_BRIGHTNESS].id ||
           gamma_size > 1;
}

bool drm_output_use_hw_color_temp(struct wlr_output *wlr_output,
                                  const struct kywc_output_state *state)
{
    if (drm_output_is_force_soft_color()) {
        return state->color_temp == 0 || state->color_temp == 6500;
    }

    struct drm_connector *conn = drm_connector_from_output(wlr_output);
    if (!conn || !conn->crtc) {
        return false;
    }

    if (state->color_feature == KYWC_OUTPUT_COLOR_FEATURE_DISABLE) {
        return true;
    }

    if (state->color_feature == KYWC_OUTPUT_COLOR_FEATURE_SOFTCOLOR ||
        state->color_feature == KYWC_OUTPUT_COLOR_FEATURE_SOFTGAMMA) {
        return false;
    }

    struct output *output = output_from_wlr_output(wlr_output);
    if (output->quirks & QUIRKS_MASK_DISABLE_GAMMA) {
        return false;
    }

    uint32_t gamma_size = drm_get_crtc_gamma_lut_size(conn->crtc);

    return state->color_temp == 0 || state->color_temp == 6500 || gamma_size > 1;
}

bool drm_output_use_hw_color_filter(struct wlr_output *wlr_output,
                                    const struct kywc_output_state *state)
{
    if (drm_output_is_force_soft_color()) {
        return !state->color_filter;
    }

    struct drm_connector *conn = drm_connector_from_output(wlr_output);
    if (!conn || !conn->crtc) {
        return false;
    }

    if (state->color_feature == KYWC_OUTPUT_COLOR_FEATURE_DISABLE) {
        return true;
    }

    if (state->color_feature == KYWC_OUTPUT_COLOR_FEATURE_SOFTCOLOR ||
        state->color_feature == KYWC_OUTPUT_COLOR_FEATURE_SOFTCTM) {
        return false;
    }

    struct output *output = output_from_wlr_output(wlr_output);
    if (output->quirks & QUIRKS_MASK_DISABLE_CTM) {
        return false;
    }

    return !state->color_filter || (state->color_filter != KYWC_OUTPUT_COLOR_FILTER_INVERT &&
                                    conn->crtc->props[DRM_CRTC_PROP_CTM].id);
}

bool wlr_output_is_hardware_color_allowed(struct wlr_output *wlr_output,
                                          const struct kywc_output_state *state)
{
    if (!wlr_output_is_drm(wlr_output)) {
        return false;
    }

    return drm_output_use_hw_brightness(wlr_output, state) &&
           drm_output_use_hw_color_temp(wlr_output, state) &&
           drm_output_use_hw_color_filter(wlr_output, state);
}

bool drm_output_yuyv_enabled(struct wlr_output *wlr_output, const struct kywc_output_state *state)
{
    struct output *output = output_from_wlr_output(wlr_output);
    if (output->quirks & QUIRKS_MASK_YUYV_MODE) {
        return true;
    }

    return false;
}
