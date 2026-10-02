/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <assert.h>

#include <cstring>
#include <string>

void testGetCapabilityList() {
    const auto length = get_capability_list_length();
    assert(length == 3);
    auto* capabilityList = new Capability[length];

    auto result = get_capability_list(capabilityList);
    assert(result == ErrorCode::SUCCEEDED);

    for (unsigned int index = 0; index < length; index++) {
        assert(capabilityList[index] == index);
    }

    delete[] capabilityList;

}

static void testGlobalSettings() {
    global_settings_reset_enabled();
    assert(global_settings_is_enabled());

    assert(global_settings_set_enabled(false));
    assert(!global_settings_is_enabled());

    global_settings_reset_enabled();
}

static void testCapabilitySettingssEnabled(Capability capability) {
    capability_settings_reset_enabled(capability);
    assert(capability_settings_is_enabled(capability));

    assert(capability_settings_set_enabled(capability, false));
    assert(!capability_settings_is_enabled(capability));

    capability_settings_reset_enabled(capability);
}

static void testCapabilitySettingssDeployPolicy(Capability capability) {
    capability_settings_reset_deploy_policy(capability);
    assert(capability_settings_get_deploy_policy(capability) == PUBLICCLOUD);

    assert(capability_settings_set_deploy_policy(capability ,LOCAL));
    assert(capability_settings_get_deploy_policy(capability) == LOCAL);

    assert(capability_settings_set_deploy_policy(capability, PRIVATECLOUD));
    assert(capability_settings_get_deploy_policy(capability) == PRIVATECLOUD);

    assert(!capability_settings_set_deploy_policy(capability, DEPLOYPOLICYINVALID));
    assert(capability_settings_get_deploy_policy(capability) == PRIVATECLOUD);

    capability_settings_reset_deploy_policy(capability);
}

static void testCapabilitySettingssLocalModel(Capability capability) {
  const auto listLength = capability_settings_get_model_list_length(capability, LOCAL);
    assert(listLength == 0);
    if (listLength != 0) {
        for (unsigned long index = 0; index < listLength; index++) {
            auto length = capability_settings_get_model_length_by_index(capability, LOCAL, index);
            assert(length != 0);
            char* localModel = new char[length];
            assert(capability_settings_get_model_by_index(capability, LOCAL, index, localModel) == SUCCEEDED);
            delete[] localModel;
        }
    }

    capability_settings_reset_model(capability, LOCAL);
    assert(capability_settings_get_model_length(capability, LOCAL) == 0);

    const std::string newLocalModel("newLocalModel");
    assert(capability_settings_set_model(capability, LOCAL, newLocalModel.c_str()) == SUCCEEDED);
    auto newlocalModelLength = capability_settings_get_model_length(capability, LOCAL);
    assert(newlocalModelLength == newLocalModel.length());
    char* localModel = new char[newlocalModelLength];
    auto result = capability_settings_get_model(capability, LOCAL, localModel);
    assert(result == SUCCEEDED);
    assert(std::strncmp(newLocalModel.c_str(), localModel, newlocalModelLength) == 0);

    delete[] localModel;

    capability_settings_reset_model(capability, LOCAL);
}

static void testCapabilitySettingssPublicCloudModel(Capability capability) {
   const auto listLength = capability_settings_get_model_list_length(capability, PUBLICCLOUD);
    assert(listLength == 2);
    if (listLength != 0) {
        for (unsigned long index = 0; index < listLength; index++) {
            auto length = capability_settings_get_model_length_by_index(capability, PUBLICCLOUD, index);
            assert(length != 0);
            char* publicCloudModel = new char[length];
            assert(capability_settings_get_model_by_index(capability, PUBLICCLOUD, index, publicCloudModel) == SUCCEEDED);
            delete[] publicCloudModel;
        }
    }

    capability_settings_reset_model(capability, PUBLICCLOUD);
    assert(capability_settings_get_model_length(capability, PUBLICCLOUD) == 0);

    const std::string newPublicCloudModel("newPublicCloudModel");
    assert(capability_settings_set_model(capability, PUBLICCLOUD, newPublicCloudModel.c_str()) == SUCCEEDED);
    auto newPublicCloudModelLength = capability_settings_get_model_length(capability, PUBLICCLOUD);
    assert(newPublicCloudModelLength == newPublicCloudModel.length());
    char* publicCloudModel = new char[newPublicCloudModelLength];
    auto result = capability_settings_get_model(capability, PUBLICCLOUD, publicCloudModel);
    assert(result == SUCCEEDED);
    assert(std::strncmp(newPublicCloudModel.c_str(), publicCloudModel, newPublicCloudModelLength) == 0);

    delete[] publicCloudModel;

    capability_settings_reset_model(capability, PUBLICCLOUD);
}

static void testCapabilitySettingssPublicCloudModelConfig(Capability capability) {
    capability_settings_reset_model_config(capability, PUBLICCLOUD, "baidu");
    assert(capability_settings_get_model_config_length(capability, PUBLICCLOUD, "baidu") == 0);

    std::string config = R"(
    {
        "apiKey": "fakeApiKey",
        "secretKey" : "fakeSecretKey"
    }
    )";
    capability_settings_set_model(capability, PUBLICCLOUD, "newPublicCloudModel");
    assert(capability_settings_set_model_config(capability, PUBLICCLOUD, "newPublicCloudModel", config.c_str()) == SUCCEEDED);
    assert(capability_settings_get_model_config_length(capability, PUBLICCLOUD, "newPublicCloudModel") != 0);

    capability_settings_reset_model(capability, PUBLICCLOUD);
    capability_settings_reset_model_config(capability, PUBLICCLOUD, "baidu");
}

static void testCapabilitySettingssPrivateCloudModel(Capability capability) {
   const auto listLength = capability_settings_get_model_list_length(capability, PRIVATECLOUD);
    assert(listLength == 0);
    if (listLength != 0) {
        for (unsigned long index = 0; index < listLength; index++) {
            auto length = capability_settings_get_model_length_by_index(capability, PRIVATECLOUD, index);
            assert(length != 0);
            char* privateCloudModel = new char[length];
            assert(capability_settings_get_model_by_index(capability, PRIVATECLOUD, index, privateCloudModel) == SUCCEEDED);
            delete[] privateCloudModel;
        }
    }

    capability_settings_reset_model(capability, PRIVATECLOUD);
    assert(capability_settings_get_model_length(capability, PRIVATECLOUD) == 0);

    const std::string newPrivateCloudModel("newPrivateCloudModel");
    assert(capability_settings_set_model(capability, PRIVATECLOUD, newPrivateCloudModel.c_str()) == SUCCEEDED);
    auto newPrivateCloudModelLength = capability_settings_get_model_length(capability, PRIVATECLOUD);
    assert(newPrivateCloudModelLength == newPrivateCloudModel.length());
    char* privateCloudModel = new char[newPrivateCloudModelLength];
    auto result = capability_settings_get_model(capability, PRIVATECLOUD, privateCloudModel);
    assert(result == SUCCEEDED);
    assert(std::strncmp(newPrivateCloudModel.c_str(), privateCloudModel, newPrivateCloudModelLength) == 0);

    delete[] privateCloudModel;

    capability_settings_reset_model(capability, PRIVATECLOUD);
}

static void testCapabilitySettingss(Capability capability) {
    testCapabilitySettingssEnabled(capability);

    testCapabilitySettingssDeployPolicy(capability);

    testCapabilitySettingssLocalModel(capability);

    testCapabilitySettingssPublicCloudModel(capability);

    testCapabilitySettingssPublicCloudModelConfig(capability);

    testCapabilitySettingssPrivateCloudModel(capability);
}

void testCapabilitiesSettings() {
    const auto length = get_capability_list_length();
    auto* capabilityList = new Capability[length];

    auto result = get_capability_list(capabilityList);
    assert(result == ErrorCode::SUCCEEDED);

    for (unsigned int index = 0; index < length; index++) {
        testCapabilitySettingss(capabilityList[index]);
    }

    delete[] capabilityList;
}

int main(int argc, char* argv[]) {
    testGetCapabilityList();

    testGlobalSettings();

    testCapabilitiesSettings();

    return 0;
}
