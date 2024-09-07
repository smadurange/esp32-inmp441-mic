#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h>
#include <esp_event.h>
#include <esp_netif.h>
#include <nvs_flash.h>

#include <driver/gpio.h>
#include <driver/i2s_std.h>

#include "wifi.h"

#define BUFLEN      1024
#define SAMP_RATE   8000

#define I2S_WS  GPIO_NUM_4
#define I2S_SD  GPIO_NUM_1
#define I2S_SCK GPIO_NUM_5

static i2s_chan_handle_t chan;

static void i2s_read_task(void *args)
{
	size_t n;
	char *buf = calloc(1, BUFLEN);

	ESP_ERROR_CHECK(i2s_channel_enable(chan));

	for (;;) {
		if (i2s_channel_read(chan, buf, BUFLEN, &n, 1000) == ESP_OK) {
			if (n > 0) {
				// todo: copy to a separate buffer and send over udp
				// esp_mqtt_client_publish(client, "snd", buf, n, 1, 0);
			}
		} else
			printf("Read Task: i2s read failed\n");
	}

	free(buf);
	vTaskDelete(NULL);
}

static inline void i2s_init(void)
{
	i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(
		I2S_NUM_AUTO,
		I2S_ROLE_MASTER);

	ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &chan));

	i2s_std_config_t std_cfg = {
		.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMP_RATE),
		.slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(
			I2S_DATA_BIT_WIDTH_24BIT,
			I2S_SLOT_MODE_MONO),
		.gpio_cfg = {
			.ws = I2S_WS,
			.din = I2S_SD,
			.bclk = I2S_SCK,
			.dout = I2S_GPIO_UNUSED,
			.mclk = I2S_GPIO_UNUSED,
			.invert_flags = {
				.ws_inv = false,
				.bclk_inv = false,
				.mclk_inv = false
			}	
		}
	};	

	std_cfg.clk_cfg.mclk_multiple = I2S_MCLK_MULTIPLE_384;

	ESP_ERROR_CHECK(i2s_channel_init_std_mode(chan, &std_cfg));
}

void app_main(void)
{
	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	wifi_connect();

	i2s_init();
	xTaskCreate(i2s_read_task, "i2s_read_task", 4096, NULL, 5, NULL);

	for (;;)
		vTaskDelay(500 / portTICK_PERIOD_MS);
}

