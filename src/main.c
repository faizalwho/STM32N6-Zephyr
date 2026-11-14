/*
 * LVGL "Hello, World!" -> fades into "Good evening!" on STM32N6570-DK (Zephyr)
 */
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/logging/log.h>
#include <lvgl.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_INF);

#define HOLD_MS  2000   /* how long the first text stays fully visible */
#define FADE_MS   800   /* fade out / fade in duration */

static lv_obj_t *label;

static void opa_exec_cb(void *obj, int32_t v)
{
	lv_obj_set_style_opa((lv_obj_t *)obj, (lv_opa_t)v, 0);
}

static void fade_out_done_cb(lv_anim_t *a)
{
	/* Swap the text while the label is invisible, then fade it back in */
	lv_label_set_text(label, "anim1");
	lv_obj_set_style_text_color(label, lv_palette_main(LV_PALETTE_ORANGE), 0);
	lv_obj_center(label);

	lv_anim_t in;

	lv_anim_init(&in);
	lv_anim_set_var(&in, label);
	lv_anim_set_exec_cb(&in, opa_exec_cb);
	lv_anim_set_values(&in, LV_OPA_TRANSP, LV_OPA_COVER);
	lv_anim_set_duration(&in, FADE_MS);
	lv_anim_set_path_cb(&in, lv_anim_path_ease_out);
	lv_anim_start(&in);
}

static void create_ui(void)
{
	lv_obj_t *scr = lv_screen_active();

	lv_obj_set_style_bg_color(scr, lv_color_hex(0x101820), 0);

	label = lv_label_create(scr);
	lv_label_set_text(label, "Hello world");
	lv_obj_set_style_text_font(label, &lv_font_montserrat_48, 0);
	lv_obj_set_style_text_color(label, lv_color_white(), 0);
	lv_obj_center(label);

	/* Fade out after HOLD_MS, then the callback swaps text and fades in */
	lv_anim_t out;

	lv_anim_init(&out);
	lv_anim_set_var(&out, label);
	lv_anim_set_exec_cb(&out, opa_exec_cb);
	lv_anim_set_values(&out, LV_OPA_COVER, LV_OPA_TRANSP);
	lv_anim_set_delay(&out, HOLD_MS);
	lv_anim_set_duration(&out, FADE_MS);
	lv_anim_set_path_cb(&out, lv_anim_path_ease_in);
	lv_anim_set_completed_cb(&out, fade_out_done_cb);
	lv_anim_start(&out);
}

int main(void)
{
	const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));

	if (!device_is_ready(display_dev)) {
		LOG_ERR("Display device not ready");
		return 0;
	}

	create_ui();
	lv_timer_handler();
	display_blanking_off(display_dev);

	while (1) {
		uint32_t sleep_ms = lv_timer_handler();

		k_msleep(MIN(sleep_ms, 33));
	}

	return 0;
}
