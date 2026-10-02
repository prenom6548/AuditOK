#include <gtest/gtest.h>
#include <gtk/gtk.h>
#include "common/eggaccelerators.h"

class EggAcceleratorsTest : public ::testing::Test {
protected:
    static void SetUpTestSuite() {
        int argc = 0;
        char **argv = nullptr;
        gtk_init(&argc, &argv);
    }

    void SetUp() override {
        // 初始化测试环境
    }

    void TearDown() override {
        // 清理测试环境
    }
};

TEST_F(EggAcceleratorsTest, ParseVirtualAccelerator) {
    const gchar* accelerator = "<Control>a";
    guint accelerator_key;
    guint* accelerator_codes;
    EggVirtualModifierType accelerator_mods;

    gboolean result = egg_accelerator_parse_virtual(accelerator, &accelerator_key, &accelerator_codes, &accelerator_mods);

    EXPECT_TRUE(result);
    EXPECT_EQ(accelerator_key, GDK_KEY_a);
    EXPECT_EQ(accelerator_mods, EGG_VIRTUAL_CONTROL_MASK);
    g_free(accelerator_codes);
}

TEST_F(EggAcceleratorsTest, ParseVirtualAcceleratorWithRelease) {
    const gchar* accelerator = "<Release>a";
    guint accelerator_key;
    guint* accelerator_codes;
    EggVirtualModifierType accelerator_mods;

    gboolean result = egg_accelerator_parse_virtual(accelerator, &accelerator_key, &accelerator_codes, &accelerator_mods);

    EXPECT_TRUE(result);
    EXPECT_EQ(accelerator_key, GDK_KEY_a);
    EXPECT_EQ(accelerator_mods, EGG_VIRTUAL_RELEASE_MASK);
    g_free(accelerator_codes);
}

TEST_F(EggAcceleratorsTest, VirtualAcceleratorName) {
    guint accelerator_key = GDK_KEY_a;
    guint keycode = 38; // Example keycode
    EggVirtualModifierType accelerator_mods = EGG_VIRTUAL_CONTROL_MASK;

    gchar* name = egg_virtual_accelerator_name(accelerator_key, keycode, accelerator_mods);

    // 考虑到 Primary 和 Control 的映射
    EXPECT_TRUE(strcmp(name, "<Control>a") == 0 || strcmp(name, "<Primary>a") == 0);
    g_free(name);
}

TEST_F(EggAcceleratorsTest, VirtualAcceleratorLabel) {
    guint accelerator_key = GDK_KEY_a;
    guint keycode = 38; // Example keycode
    EggVirtualModifierType accelerator_mods = EGG_VIRTUAL_CONTROL_MASK;

    gchar* label = egg_virtual_accelerator_label(accelerator_key, keycode, accelerator_mods);

    EXPECT_STREQ(label, "Ctrl+A");
    g_free(label);
}

TEST_F(EggAcceleratorsTest, ResolveVirtualModifiers) {
    GdkModifierType concrete_mods;
    egg_keymap_resolve_virtual_modifiers(NULL, EGG_VIRTUAL_CONTROL_MASK, &concrete_mods);

    EXPECT_EQ(concrete_mods, GDK_CONTROL_MASK);
}

TEST_F(EggAcceleratorsTest, VirtualizeModifiers) {
    EggVirtualModifierType virtual_mods;
    egg_keymap_virtualize_modifiers(NULL, GDK_CONTROL_MASK, &virtual_mods);

    EXPECT_EQ(virtual_mods, EGG_VIRTUAL_CONTROL_MASK);
}
