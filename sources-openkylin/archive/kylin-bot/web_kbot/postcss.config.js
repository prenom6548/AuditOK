import cascadeLayers from '@csstools/postcss-cascade-layers';
import postcssPresetEnv from 'postcss-preset-env';

/** Qt 5.15.19 WebEngine ≈ Chromium 87 — flatten @layer and lower CSS for embedded WebView. */
export default {
  plugins: [
    cascadeLayers(),
    postcssPresetEnv({
      stage: 3,
      browsers: 'chrome >= 87, safari >= 14, firefox >= 78',
      features: {
        'cascade-layers': false,
      },
    }),
  ],
};
