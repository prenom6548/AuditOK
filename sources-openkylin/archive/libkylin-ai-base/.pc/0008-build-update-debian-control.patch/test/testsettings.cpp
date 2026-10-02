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

#include <assert.h>

#include "settings/globalsettings.h"

using namespace aisdk;

static void testGlobalSettings() {
    GlobalSettings::getInstance().resetEnabled();
    assert(GlobalSettings::getInstance().isEnabled());

    assert(GlobalSettings::getInstance().setEnabled(false));
    assert(!GlobalSettings::getInstance().isEnabled());

    GlobalSettings::getInstance().resetEnabled();
}

static void testChildSettings(ChildSettings& childSettings) {
    childSettings.resetEnabled();
    assert(childSettings.isEnabled());

    assert(childSettings.setEnabled(false));
    assert(!childSettings.isEnabled());

    childSettings.resetEnabled();

    childSettings.resetDeployPolicy();
    assert(childSettings.getDeployPolicy() == PUBLICCLOUD);

    assert(childSettings.setDeployPolicy(LOCAL));
    assert(childSettings.getDeployPolicy() == LOCAL);

    assert(childSettings.setDeployPolicy(PRIVATECLOUD));
    assert(childSettings.getDeployPolicy() == PRIVATECLOUD);

    assert(!childSettings.setDeployPolicy(DEPLOYPOLICYINVALID));
    assert(childSettings.getDeployPolicy() == PRIVATECLOUD);

    childSettings.resetDeployPolicy();

    childSettings.resetModel(LOCAL);
    assert(childSettings.getModel(LOCAL).empty());

    assert(childSettings.setModel(LOCAL, "newLocalModel"));
    assert(childSettings.getModel(LOCAL) == "newLocalModel");

    childSettings.resetModel(LOCAL);

    childSettings.resetModel(PUBLICCLOUD);
    assert(childSettings.getModel(PUBLICCLOUD).empty());

    assert(childSettings.setModel(PUBLICCLOUD, "newPublicCloudModel"));
    assert(childSettings.getModel(PUBLICCLOUD) == "newPublicCloudModel");

    childSettings.resetModel(PUBLICCLOUD);

    childSettings.resetModelConfig(PUBLICCLOUD, "newPublicCloudModel");
    assert(childSettings.getModelConfig(PUBLICCLOUD, "newPublicCloudModel").empty());

    std::string config = R"(
    {
        "apiKey": "fakeApiKey",
        "secretKey" : "fakeSecretKey"
    }
    )";
    childSettings.setModel(PUBLICCLOUD, "newPublicCloudModel");
    assert(childSettings.setModelConfig(PUBLICCLOUD, "newPublicCloudModel", config));
    assert(!childSettings.getModelConfig(PUBLICCLOUD, "newPublicCloudModel").empty());

    childSettings.resetModel(PUBLICCLOUD);
    childSettings.resetModelConfig(PUBLICCLOUD, "newPublicCloudModel");

    childSettings.resetModel(PRIVATECLOUD);
    assert(childSettings.getModel(PRIVATECLOUD).empty());

    assert(childSettings.setModel(PRIVATECLOUD, "newPrivateCloudModel"));
    assert(childSettings.getModel(PRIVATECLOUD) == "newPrivateCloudModel");

    childSettings.resetModel(PRIVATECLOUD);
}

static void testTextSettings() {
    testChildSettings(GlobalSettings::getInstance().getChildSettings(NLP));
}

static void testSpeechSettings() {
    testChildSettings(GlobalSettings::getInstance().getChildSettings(SPEECH));
}

static void testVisionSettings() {
    testChildSettings(GlobalSettings::getInstance().getChildSettings(VISION));
}

int main(int argc, char* argv[]) {
    testGlobalSettings();

    testTextSettings();

    testSpeechSettings();

    testVisionSettings();

    return 0;
}