// SPDX-License-Identifier: GPL-2.0-only
/*
 * Driver for the Samsung s6e3fc3_6a panel.
 *
 * Copyright (c) 2022-2024, The Linux Foundation. All rights reserved.
 * Generated with linux-mdss-dsi-panel-driver-generator from vendor device tree:
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 */

#include <linux/backlight.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/regulator/consumer.h>

#include <video/mipi_display.h>

#include <drm/drm_mipi_dsi.h>
#include <drm/drm_modes.h>
#include <drm/drm_panel.h>

struct s6e3fc3_6a_panel {
	struct drm_panel panel;
	struct mipi_dsi_device *dsi;
	struct gpio_desc *reset_gpio;
	struct regulator *vci;
	struct regulator *vddi;
};

/* DSC Picture Parameter Set for 1080x2400, 8bpc, 2 slices of 540 pixels */
static const u8 pps_setting[] = {
	0x11, 0x00, 0x00, 0x89, 0x30, 0x80, 0x09, 0x60,
	0x04, 0x38, 0x00, 0x30, 0x02, 0x1C, 0x02, 0x1C,
	0x02, 0x00, 0x02, 0x0E, 0x00, 0x20, 0x04, 0xA6,
	0x00, 0x07, 0x00, 0x0C, 0x02, 0x0B, 0x02, 0x1F,
	0x18, 0x00, 0x10, 0xF0, 0x03, 0x0C, 0x20, 0x00,
	0x06, 0x0B, 0x0B, 0x33, 0x0E, 0x1C, 0x2A, 0x38,
	0x46, 0x54, 0x62, 0x69, 0x70, 0x77, 0x79, 0x7B,
	0x7D, 0x7E, 0x01, 0x02, 0x01, 0x00, 0x09, 0x40,
	0x09, 0xBE, 0x19, 0xFC, 0x19, 0xFA, 0x19, 0xF8,
	0x1A, 0x38, 0x1A, 0x78, 0x1A, 0xB6, 0x2A, 0xF6,
	0x2B, 0x34, 0x2B, 0x74, 0x3B, 0x74, 0x6B, 0xF4,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

static inline struct s6e3fc3_6a_panel *
to_s6e3fc3_6a_panel(struct drm_panel *panel)
{
	return container_of(panel, struct s6e3fc3_6a_panel, panel);
}

static void s6e3fc3_6a_panel_reset(struct s6e3fc3_6a_panel *ctx)
{
	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	usleep_range(1000, 2000);
	gpiod_set_value_cansleep(ctx->reset_gpio, 0);
	usleep_range(10000, 11000);
}

static int s6e3fc3_6a_panel_on(struct mipi_dsi_device *dsi)
{
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = dsi };

	mipi_dsi_dcs_exit_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);

	mipi_dsi_dcs_set_tear_on_multi(&dsi_ctx, MIPI_DSI_DCS_TEAR_MODE_VBLANK);

	/* CASET: column address set */
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_COLUMN_ADDRESS,
				     0x00, 0x00, 0x04, 0x37);
	/* PASET: page address set */
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_SET_PAGE_ADDRESS,
				     0x00, 0x00, 0x09, 0x5f);

	/* Test key enable */
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0x5a, 0x5a);

	/* FQ CON setting */
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0, 0x27, 0xf2);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf2, 0x80);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf7, 0x07);

	/* IRC setting */
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xb0, 0x03, 0x8f);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x8f, 0x25);

	/* Test key disable */
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xf0, 0xa5, 0xa5);

	/* DSC configuration */
	mipi_dsi_compression_mode_multi(&dsi_ctx, true);
	mipi_dsi_generic_write_multi(&dsi_ctx, pps_setting, sizeof(pps_setting));
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0xc2, 0x14);
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, 0x9d, 0x01);

	/* Dimming and brightness control */
	mipi_dsi_dcs_write_seq_multi(&dsi_ctx, MIPI_DCS_WRITE_CONTROL_DISPLAY,
				     0x20);

	mipi_dsi_dcs_set_display_on_multi(&dsi_ctx);

	return dsi_ctx.accum_err;
}

static int s6e3fc3_6a_panel_prepare(struct drm_panel *panel)
{
	struct s6e3fc3_6a_panel *ctx = to_s6e3fc3_6a_panel(panel);
	int ret;

	ret = regulator_enable(ctx->vci);
	if (ret < 0)
		return ret;

	if (ctx->vddi) {
		ret = regulator_enable(ctx->vddi);
		if (ret < 0) {
			regulator_disable(ctx->vci);
			return ret;
		}
	}

	s6e3fc3_6a_panel_reset(ctx);

	ret = s6e3fc3_6a_panel_on(ctx->dsi);
	if (ret < 0) {
		gpiod_set_value_cansleep(ctx->reset_gpio, 1);
		if (ctx->vddi)
			regulator_disable(ctx->vddi);
		regulator_disable(ctx->vci);
	}

	return ret;
}

static int s6e3fc3_6a_panel_unprepare(struct drm_panel *panel)
{
	struct s6e3fc3_6a_panel *ctx = to_s6e3fc3_6a_panel(panel);

	gpiod_set_value_cansleep(ctx->reset_gpio, 1);
	if (ctx->vddi)
		regulator_disable(ctx->vddi);
	regulator_disable(ctx->vci);

	return 0;
}

static int s6e3fc3_6a_panel_disable(struct drm_panel *panel)
{
	struct s6e3fc3_6a_panel *ctx = to_s6e3fc3_6a_panel(panel);
	struct mipi_dsi_device *dsi = ctx->dsi;
	struct mipi_dsi_multi_context dsi_ctx = { .dsi = dsi };

	mipi_dsi_dcs_set_display_off_multi(&dsi_ctx);
	mipi_dsi_dcs_enter_sleep_mode_multi(&dsi_ctx);
	mipi_dsi_msleep(&dsi_ctx, 120);

	return dsi_ctx.accum_err;
}

static const struct drm_display_mode s6e3fc3_6a_panel_mode = {
	.clock = (1080 + 32 + 12 + 26) * (2400 + 12 + 4 + 26) * 60 / 1000,
	.hdisplay = 1080,
	.hsync_start = 1080 + 32,
	.hsync_end = 1080 + 32 + 12,
	.htotal = 1080 + 32 + 12 + 26,
	.vdisplay = 2400,
	.vsync_start = 2400 + 12,
	.vsync_end = 2400 + 12 + 4,
	.vtotal = 2400 + 12 + 4 + 26,
	.width_mm = 64,
	.height_mm = 142,
};

static int s6e3fc3_6a_panel_get_modes(struct drm_panel *panel,
				      struct drm_connector *connector)
{
	struct drm_display_mode *mode;

	mode = drm_mode_duplicate(connector->dev, &s6e3fc3_6a_panel_mode);
	if (!mode)
		return -ENOMEM;

	drm_mode_set_name(mode);

	mode->type = DRM_MODE_TYPE_DRIVER | DRM_MODE_TYPE_PREFERRED;
	connector->display_info.width_mm = mode->width_mm;
	connector->display_info.height_mm = mode->height_mm;
	drm_mode_probed_add(connector, mode);

	return 1;
}

static const struct drm_panel_funcs s6e3fc3_6a_panel_funcs = {
	.prepare = s6e3fc3_6a_panel_prepare,
	.unprepare = s6e3fc3_6a_panel_unprepare,
	.disable = s6e3fc3_6a_panel_disable,
	.get_modes = s6e3fc3_6a_panel_get_modes,
};

static int s6e3fc3_6a_panel_bl_update_status(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness = backlight_get_brightness(bl);
	int ret;

	ret = mipi_dsi_dcs_set_display_brightness_large(dsi, brightness);
	if (ret < 0)
		return ret;

	return 0;
}

static int s6e3fc3_6a_panel_bl_get_brightness(struct backlight_device *bl)
{
	struct mipi_dsi_device *dsi = bl_get_data(bl);
	u16 brightness;
	int ret;

	ret = mipi_dsi_dcs_get_display_brightness_large(dsi, &brightness);
	if (ret < 0)
		return ret;

	return brightness;
}

static const struct backlight_ops s6e3fc3_6a_panel_bl_ops = {
	.update_status = s6e3fc3_6a_panel_bl_update_status,
	.get_brightness = s6e3fc3_6a_panel_bl_get_brightness,
};

static struct backlight_device *
s6e3fc3_6a_panel_create_backlight(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	const struct backlight_properties props = {
		.type = BACKLIGHT_RAW,
		.brightness = 1023,
		.max_brightness = 2047,
	};

	return devm_backlight_device_register(dev, dev_name(dev), dev, dsi,
					      &s6e3fc3_6a_panel_bl_ops, &props);
}

static int s6e3fc3_6a_panel_probe(struct mipi_dsi_device *dsi)
{
	struct device *dev = &dsi->dev;
	struct s6e3fc3_6a_panel *ctx;
	int ret;

	ctx = devm_drm_panel_alloc(dev, struct s6e3fc3_6a_panel, panel,
				   &s6e3fc3_6a_panel_funcs,
				   DRM_MODE_CONNECTOR_DSI);
	if (IS_ERR(ctx))
		return PTR_ERR(ctx);

	ctx->reset_gpio = devm_gpiod_get(dev, "reset", GPIOD_OUT_HIGH);
	if (IS_ERR(ctx->reset_gpio))
		return dev_err_probe(dev, PTR_ERR(ctx->reset_gpio),
				     "Failed to get reset-gpios\n");

	ctx->vci = devm_regulator_get(dev, "vci");
	if (IS_ERR(ctx->vci))
		return dev_err_probe(dev, PTR_ERR(ctx->vci),
				     "Failed to get vci regulator\n");

	ctx->vddi = devm_regulator_get_optional(dev, "vddi");
	if (IS_ERR(ctx->vddi)) {
		if (PTR_ERR(ctx->vddi) == -ENODEV) {
			ctx->vddi = NULL;
		} else {
			return dev_err_probe(dev, PTR_ERR(ctx->vddi),
					     "Failed to get vddi regulator\n");
		}
	}

	ctx->dsi = dsi;
	mipi_dsi_set_drvdata(dsi, ctx);

	dsi->lanes = 4;
	dsi->format = MIPI_DSI_FMT_RGB888;
	dsi->mode_flags = MIPI_DSI_CLOCK_NON_CONTINUOUS | MIPI_DSI_MODE_LPM;

	ctx->panel.prepare_prev_first = true;

	ctx->panel.backlight = s6e3fc3_6a_panel_create_backlight(dsi);
	if (IS_ERR(ctx->panel.backlight))
		return dev_err_probe(dev, PTR_ERR(ctx->panel.backlight),
				     "Failed to create backlight\n");

	drm_panel_add(&ctx->panel);

	ret = mipi_dsi_attach(dsi);
	if (ret < 0) {
		dev_err(dev, "Failed to attach to DSI host: %d\n", ret);
		drm_panel_remove(&ctx->panel);
		return ret;
	}

	return 0;
}

static void s6e3fc3_6a_panel_remove(struct mipi_dsi_device *dsi)
{
	struct s6e3fc3_6a_panel *ctx = mipi_dsi_get_drvdata(dsi);
	int ret;

	ret = mipi_dsi_detach(dsi);
	if (ret < 0)
		dev_err(&dsi->dev, "Failed to detach from DSI host: %d\n", ret);

	drm_panel_remove(&ctx->panel);
}

static const struct of_device_id s6e3fc3_6a_panel_of_match[] = {
	{ .compatible = "samsung,s6e3fc3_6a" },
	{ .compatible = "samsung,s6e3fc3-6a" },
	{ /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, s6e3fc3_6a_panel_of_match);

static struct mipi_dsi_driver s6e3fc3_6a_panel_driver = {
	.probe = s6e3fc3_6a_panel_probe,
	.remove = s6e3fc3_6a_panel_remove,
	.driver = {
		.name = "panel-samsung-s6e3fc3_6a",
		.of_match_table = s6e3fc3_6a_panel_of_match,
	},
};
module_mipi_dsi_driver(s6e3fc3_6a_panel_driver);

MODULE_AUTHOR("Pedro Alves <pta2002@pta2002.com>");
MODULE_DESCRIPTION("DRM driver for Samsung S6E3FC3-6A command mode DSI panel");
MODULE_LICENSE("GPL");
