/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2024-2024. All rights reserved.
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 */
#ifndef __BM_SPI_H__
#define __BM_SPI_H__
/**
 * @defgroup bm_spi bm_spi
 */

#define CORE_SYS_SPI0_INTID 146
#define CORE_SYS_SPI1_INTID 171

#define USE_SPI1_SFC_PIN 1
#define SPI_TX_FIFO_SIZE 32
#define SPI_RX_FIFO_SIZE 32

typedef enum {
    HISPI_IDLE_CLOCK_POLARITYLOW = 0, /* Output clock polarity is low when idle */
    HISPI_IDLE_CLOCK_POLARITYHIGH,    /* Output clock polarity high when idle */
    HISPI_IDLE_CLOCK_POLARITYERR      /* Output clock polarity error when idle */
} bm_spi_clock_polarity;

typedef enum {
    HISPI_CLOCK_EDGEUP = 0, /* Data transfer starts on the chip-select falling edge */
    HISPI_CLOCK_EDGEDOWN,   /* Data transfer starts on the first clock edge after the chip select is valid */
    HISPI_CLOCK_EDGEERR,    /* Serial clock phase error */
} bm_spi_clock_edge;

typedef enum {
    HISPI_FRAME_LENGTH_4_BIT = 0, /* The length of the data frame is 4 bits. */
    HISPI_FRAME_LENGTH_8_BIT,     /* The length of the data frame is 8 bits. */
    HISPI_FRAME_LENGTH_16_BIT,    /* The length of the data frame is 16 bits. */
} bm_spi_frame_length;

typedef enum {
    HISPI_CS0_SELECT = 1, /* cs0 select. */
    HISPI_CS1_SELECT = 2,     /* cs1 select. */
    HISPI_CS_ALL_SELECT = 3,    /* all select. */
} bm_spi_cs_select;


typedef struct {
    unsigned int spi_id;
    unsigned int baud;
    bm_spi_cs_select cs_map; /* Each bit is 1, indicating that the CS is enabled. */
    bm_spi_frame_length frame_len;
    bm_spi_clock_edge clk_edge;
    bm_spi_clock_polarity clk_pol;
    unsigned int bm_spi_rx_threshold;
} bm_spi_cfg;

typedef struct {
    unsigned int spi_id;
    unsigned int cs_id;
} bm_spi_handle;

typedef struct {
    unsigned char *r_buf;
    unsigned int r_len;
    unsigned char *w_buf;
    unsigned int w_len;
    unsigned int time_out;
} bm_spi_data;

typedef enum {
    SPI_WRITE_OK = 0,
    SPI_WRITE_FAIL,
    SPI_READ_OK,
    SPI_READ_FAIL,
    SPI_TRANSMIT_BUTT,
} bm_spi_transmit_async_t;
typedef void (*bm_spi_callback)(int id, bm_spi_transmit_async_t);

/**
 *
 * @ingroup bm_spi
 * @brief SPI initialization.
 *
 * @par description:
 * SPI initialization.
 *
 * @param cfg [in]: bm_spi_cfg
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_spi_init(const bm_spi_cfg *cfg);

/**
 *
 * @ingroup bm_spi
 * @brief SPI deinitialization.
 *
 * @par description:
 * SPI deinitialization.
 *
 * @param spi_id [in]: spi_id
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_spi_deinit(unsigned int spi_id);

/**
 *
 * @ingroup bm_spi
 * @brief SPI clock config.
 *
 * @par description:
 * SPI clock config.
 *
 * @param handle [in]: bm_spi_handle
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_spi_config_io_clock(const bm_spi_handle *handle);

/**
 *
 * @ingroup bm_spi
 * @brief SPI io reuse config.
 *
 * @par description:
 * SPI io reuse config.
 *
 * @param handle [in]: bm_spi_handle
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_spi_config_io_reuse(const bm_spi_handle *handle);

/**
 *
 * @ingroup bm_spi
 * @brief SPI config.
 *
 * @par description:
 * SPI config.
 *
 * @param cfg [in]: bm_spi_cfg
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_spi_config(const bm_spi_cfg *cfg);

/**
 *
 * @ingroup bm_spi
 * @brief SPI write data synchronously.
 *
 * @par description:
 * SPI write data synchronously.
 *
 * @param handle [in]: bm_spi_handle
 * @param w_data [in]: write data
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_spi_write(const bm_spi_handle *handle, bm_spi_data *w_data);

/**
 *
 * @ingroup bm_spi
 * @brief SPI read data synchronously.
 *
 * @par description:
 * SPI read data synchronously.
 *
 * @param handle [in]: bm_spi_handle
 * @param r_buf [in]: read data
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_spi_read(const bm_spi_handle *handle, bm_spi_data *r_data);

/**
 *
 * @ingroup bm_spi
 * @brief SPI write read data synchronously.
 *
 * @par description:
 * SPI write read data synchronously.
 *
 * @param handle [in]: bm_spi_handle
 * @param w_r_data [in]: write read data
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_spi_write_read(const bm_spi_handle *handle, bm_spi_data *w_r_data);

/**
 *
 * @ingroup bm_spi
 * @brief SPI write data dma.
 *
 * @par description:
 * SPI write data dma.
 *
 * @param handle [in]: bm_spi_handle
 * @param w_data [in]: write data
 * @param callback [in]: bm_spi_callback
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_spi_write_dma(const bm_spi_handle *handle, bm_spi_data *w_data, bm_spi_callback callback);

/**
 *
 * @ingroup bm_spi
 * @brief SPI read data dma.
 *
 * @par description:
 * SPI read data dma.
 *
 * @param handle [in]: bm_spi_handle
 * @param r_buf [in]: read data
 * @param callback [in]: bm_spi_callback
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_spi_read_dma(const bm_spi_handle *handle, bm_spi_data *r_data, bm_spi_callback callback);

/**
 *
 * @ingroup bm_spi
 * @brief SPI write data asynchronously.
 *
 * @par description:
 * SPI write data asynchronously.
 *
 * @param handle [in]: bm_spi_handle
 * @param w_r_data [in]: write data
 * @param callback [in]: bm_spi_callback
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_spi_write_async(const bm_spi_handle *handle, bm_spi_data *w_r_data, bm_spi_callback callback);

/**
 *
 * @ingroup bm_spi
 * @brief SPI read data asynchronously.
 *
 * @par description:
 * SPI read data asynchronously.
 *
 * @param handle [in]: bm_spi_handle
 * @param r_buf [in]: read data
 * @param callback [in]: bm_spi_callback
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_spi_read_async(const bm_spi_handle *handle, bm_spi_data *r_data, bm_spi_callback callback);

/**
 *
 * @ingroup bm_spi
 * @brief SPI write read data asynchronously.
 *
 * @par description:
 * SPI write read data asynchronously.
 *
 * @param handle [in]: bm_spi_handle
 * @param w_r_data [in]: write read data
 * @param callback [in]: bm_spi_callback
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_spi_write_read_async(const bm_spi_handle *handle, bm_spi_data *w_r_data, bm_spi_callback callback);
#endif /* __BM_SPI_H__ */

