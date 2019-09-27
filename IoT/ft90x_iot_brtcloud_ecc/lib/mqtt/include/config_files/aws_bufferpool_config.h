/*
 * Amazon FreeRTOS V1.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/**
 * @file aws_bufferpool_config.h
 * @brief Buffer Pool config options.
 */

#ifndef _AWS_BUFFER_POOL_CONFIG_H_
#define _AWS_BUFFER_POOL_CONFIG_H_

/**
 * @brief The number of buffers in the static buffer pool.
 */
#define bufferpoolconfigNUM_BUFFERS    ( 2 )

/**
 * @brief The size of each buffer in the static buffer pool.
 * Increase this buffer as necessary
 * Note that this buffer is for sending MQTT packets only
 * This buffer size has been reduced for memory footprint purposes
 */
#define bufferpoolconfigBUFFER_DATA_SIZE    ( 128+64+32 )

/**
 * @brief The size of each buffer in the static buffer pool.
 */
#define bufferpoolconfigBUFFER_SIZE    ( bufferpoolconfigBUFFER_DATA_SIZE + 64 )

#endif /* _AWS_BUFFER_POOL_CONFIG_H_ */
