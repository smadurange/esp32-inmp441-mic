#include <string.h>
#include <sys/param.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_event.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_system.h>
#include <nvs_flash.h>

#include <driver/gpio.h>
#include <driver/i2s_std.h>

#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>

#include "wifi.h"

#define BUFLEN	      1024
#define SAMPLE_RATE   8000

#define I2S_WS  GPIO_NUM_4
#define I2S_SD  GPIO_NUM_1
#define I2S_SCK GPIO_NUM_5

static i2s_chan_handle_t chan;
static char *data = calloc(1, BUFLEN + 1);

static void i2s_read_task(void *args)
{
	size_t n;
	char *buf = calloc(1, BUFLEN);

	ESP_ERROR_CHECK(i2s_channel_enable(chan));

	for (;;) {
		if (i2s_channel_read(chan, buf, BUFLEN, &n, 1000) == ESP_OK) {
			if (n > 0) {
				memcpy(data, buf, n);
				data[n] = '\0';
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
		I2S_NUM_AUTO, I2S_ROLE_MASTER);

	ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, NULL, &chan));

	i2s_std_config_t std_cfg = {
		.clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(SAMP_RATE),
		.slot_cfg = I2S_STD_PHILIPS_SLOT_DEFAULT_CONFIG(
			I2S_DATA_BIT_WIDTH_24BIT, I2S_SLOT_MODE_MONO),
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

static void udp_send_task(void *args)
{
	const int family = AF_INET;
	const int port = CONFIG_UDP_PORT;
	const char[] ip_addr = CONFIG_UDP_ADDR;
	
	int sock;
	struct timeval timeout;
	struct sockaddr_in addr;

	addr_family = AF_INET;

	dest_addr.sin_addr.s_addr = inet_addr(ip_addr);
	dest_addr.sin_family = family;
	dest_addr.sin_port = htons(port);	

	int sock = socket(family, SOCK_DGRAM, IPPROTO_IP);	

	if (sock < 0) {
		ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
		break;
	}

	timeout.tv_sec = 10;
	timeout.tv_usec = 0;
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

	ESP_LOGI(TAG, "Socket created, sending to %s:%d", ip_addr, port);

	for (;;) {
		int err = sendto(sock, data, strlen(data), 0,
			(struct sockaddr *)&addr, sizeof addr);

		if (err < 0) {
			ESP_LOGE(TAG, "Error occurred during sending: errno %d", errno);
			break;
		}

		if (sock != -1) {
			ESP_LOGE(TAG, "Shutting down socket and restarting...");
			shutdown(sock, 0);
			close(sock);
		}
	}

	vTaskDelete(NULL);
}

void app_main(void)
{
	ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(esp_event_loop_create_default());

	wifi_connect();
	i2s_init();

	xTaskCreate(i2s_read_task, "i2s_read_task", 4096, NULL, 5, NULL);
	xTaskCreate(udp_send_task, "udp_send_task", 4096, NULL, 5, NULL);

	for (;;)
		vTaskDelay(500 / portTICK_PERIOD_MS);
}

