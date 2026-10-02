/*
 * libkysdk-system's Library
 *
 * Copyright (C) 2025, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yunhe Liu <liuyunhe@kylinos.cn>
 *
 */


// clipboard_test.c
#include "../libkydataexchange.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <X11/Xlib.h>

#ifndef STRNLEN_MAX
#define STRNLEN_MAX 1024 * 1024 * 1024
#endif

// Test callback function
static void on_clipboard_change(void* user_data) {
    printf("Clipboard content changed! User data: %s\n", (char*)user_data);
}

// Test text operations
static bool test_text_operations() {
    printf("\n=== Testing Text Operations ===\n");
    
    const char* test_text = "Hello, Clipboard!";
    ClipboardOperationResult result;
    
    // Set text data
    result = kdk_exchange_set_data(CLIPBOARD_TYPE_TEXT, test_text, strnlen(test_text, STRNLEN_MAX) + 1);
    if (result != CLIPBOARD_OPERATION_SUCCESS) {
        printf("Failed to set text data\n");
        return false;
    }
    printf("Successfully set text: %s\n", test_text);
    
    // Get text data
    ClipboardDataType type;
    void* data;
    result = kdk_exchange_get_data(&type, &data);
    if (result != CLIPBOARD_OPERATION_SUCCESS) {
        printf("Failed to get text data\n");
        return false;
    }
    
    if (type != CLIPBOARD_TYPE_TEXT) {
        printf("Wrong data type received\n");
        free(data);
        return false;
    }
    
    printf("Retrieved text: %s\n", (char*)data);
    bool success = strcmp(test_text, (char*)data) == 0;
    free(data);
    
    return success;
}

// Test image operations
static bool test_image_operations() {
    printf("\n=== Testing Image Operations ===\n");
    
    // Create a simple test image (2x2 pixels, RGBA)
    ClipboardImage test_image = {
        .width = 2,
        .height = 2,
        .bytes_per_pixel = 4,
        .data = malloc(2 * 2 * 4)
    };
    
    if (!test_image.data) {
        printf("Failed to allocate image memory\n");
        return false;
    }
    
    // Fill with a simple pattern
    unsigned char* pixels = test_image.data;
    for (size_t i = 0; i < 16; i++) {
        pixels[i] = i % 256;
    }
    
    // Set image data
    ClipboardOperationResult result = kdk_exchange_set_data(
        CLIPBOARD_TYPE_IMAGE,
        &test_image,
        sizeof(ClipboardImage)
    );
    
    if (result != CLIPBOARD_OPERATION_SUCCESS) {
        printf("Failed to set image data\n");
        free(test_image.data);
        return false;
    }
    printf("Successfully set image data\n");
    
    // Get image data
    ClipboardDataType type;
    void* data;
    result = kdk_exchange_get_data(&type, &data);
    if (result != CLIPBOARD_OPERATION_SUCCESS) {
        printf("Failed to get image data\n");
        free(test_image.data);
        return false;
    }
    
    if (type != CLIPBOARD_TYPE_IMAGE) {
        printf("Wrong data type received\n");
        free(test_image.data);
        free(data);
        return false;
    }
    
    ClipboardImage* retrieved_image = (ClipboardImage*)data;
    printf("Retrieved image: %zux%zu, %zu bytes per pixel\n",
           retrieved_image->width,
           retrieved_image->height,
           retrieved_image->bytes_per_pixel);
    
    // Compare image data
    bool success = (
        retrieved_image->width == test_image.width &&
        retrieved_image->height == test_image.height &&
        retrieved_image->bytes_per_pixel == test_image.bytes_per_pixel &&
        memcmp(retrieved_image->data, test_image.data, 
               test_image.width * test_image.height * test_image.bytes_per_pixel) == 0
    );
    
    free(test_image.data);
    free(retrieved_image->data);
    free(retrieved_image);
    
    return success;
}

// Test clipboard state
static bool test_clipboard_state() {
    printf("\n=== Testing Clipboard State ===\n");
    
    // Test empty state
    if (kdk_exchange_clear()!= 0) {
        printf("Failed to clear clipboard\n");
        return false;
    }
    
    if (!kdk_exchange_is_empty()) {
        printf("Clipboard should be empty after clear\n");
        return false;
    }
    printf("Successfully cleared clipboard\n");
    
    // Test non-empty state
    const char* test_text = "Test";
    if (kdk_exchange_set_data(CLIPBOARD_TYPE_TEXT, test_text, strnlen(test_text, STRNLEN_MAX) + 1) 
        != CLIPBOARD_OPERATION_SUCCESS) {
        printf("Failed to set test data\n");
        return false;
    }
    
    if (kdk_exchange_is_empty()) {
        printf("Clipboard should not be empty after setting data\n");
        return false;
    }
    printf("Successfully tested clipboard state\n");
    
    return true;
}

// Test callback registration
static bool test_callbacks() {
    printf("\n=== Testing Callbacks ===\n");
    
    char user_data[] = "Test Callback Data";
    
    // Register callback
    ClipboardOperationResult result = kdk_exchange_register_callback(
        on_clipboard_change,
        user_data
    );
    if (result != CLIPBOARD_OPERATION_SUCCESS) {
        printf("Failed to register callback\n");
        return false;
    }
    printf("Successfully registered callback\n");
    
    // Test callback triggering
    const char* test_text = "Callback Test";
    result = kdk_exchange_set_data(CLIPBOARD_TYPE_TEXT, test_text, strnlen(test_text, STRNLEN_MAX) + 1);
    if (result != CLIPBOARD_OPERATION_SUCCESS) {
        printf("Failed to set test data for callback\n");
        return false;
    }
    
    // Give some time for the callback to be triggered
    usleep(100000);
    
    // Unregister callback
    result = kdk_exchange_unregister_callback(on_clipboard_change);
    if (result != CLIPBOARD_OPERATION_SUCCESS) {
        printf("Failed to unregister callback\n");
        return false;
    }
    printf("Successfully unregistered callback\n");
    
    return true;
}

int main() {
    // Initialize clipboard
    if (!kdk_exchange_init()) {
        printf("Failed to initialize clipboard\n");
        return 1;
    }
    printf("Clipboard initialized successfully\n");
    
    // Run tests
    bool success = true;
    
    if (!test_text_operations()) {
        printf("Text operations test failed\n");
        success = false;
    }
    
    if (!test_image_operations()) {
        printf("Image operations test failed\n");
        success = false;
    }
    
    if (!test_clipboard_state()) {
        printf("Clipboard state test failed\n");
        success = false;
    }
    
    if (!test_callbacks()) {
        printf("Callback test failed\n");
        success = false;
    }
    
    // Cleanup
    kdk_exchange_cleanup();
    printf("\nClipboard cleaned up\n");
    
    return success ? 0 : 1;
}