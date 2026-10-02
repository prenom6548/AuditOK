.pragma library

function getExcludeFields() {
    return ["AltTriggerKeys",
        "EnumerateBackwardKeys",
        "EnumerateGroupForwardKeys",
        "EnumerateGroupBackwardKeys",
        "EnumerateSkipFirst",
        "ActivateKeys",
        "DeactivateKeys",
        "resetStateWhenFocusIn",
        "TogglePreedit",
        "ModifierOnlyKeyTimeout",
        "PrevPage",
        "NextPage",
        "PrevCandidate",
        "NextCandidate"]
}


var DynamicTranslations = {
    "ActiveByDefault": QT_TRANSLATE_NOOP("DynamicContext", "ActiveByDefault"),
    "AllowInputMethodForPassword": QT_TRANSLATE_NOOP("DynamicContext", "AllowInputMethodForPassword"),
    "CompactInputMethodInformation": QT_TRANSLATE_NOOP("DynamicContext", "CompactInputMethodInformation"),
    "CustomXkbOption": QT_TRANSLATE_NOOP("DynamicContext", "CustomXkbOption"),
    "DefaultPageSize": QT_TRANSLATE_NOOP("DynamicContext", "DefaultPageSize"),
    "DisabledAddons": QT_TRANSLATE_NOOP("DynamicContext", "DisabledAddons"),
    "EnabledAddons": QT_TRANSLATE_NOOP("DynamicContext", "EnabledAddons"),
    "OverrideXkbOption": QT_TRANSLATE_NOOP("DynamicContext", "OverrideXkbOption"),
    "PreeditEnabledByDefault": QT_TRANSLATE_NOOP("DynamicContext", "PreeditEnabledByDefault"),
    "PreloadInputMethod": QT_TRANSLATE_NOOP("DynamicContext", "PreloadInputMethod"),
    "ShareInputState": QT_TRANSLATE_NOOP("DynamicContext", "ShareInputState"),
    "ShowFirstInputMethodInformation": QT_TRANSLATE_NOOP("DynamicContext", "ShowFirstInputMethodInformation"),
    "ShowInputMethodInformation": QT_TRANSLATE_NOOP("DynamicContext", "ShowInputMethodInformation"),
    "ShowPreeditForPassword": QT_TRANSLATE_NOOP("DynamicContext", "ShowPreeditForPassword"),
    "resetStateWhenFocusIn": QT_TRANSLATE_NOOP("DynamicContext", "resetStateWhenFocusIn"),
    "showInputMethodInformationWhenFocusIn": QT_TRANSLATE_NOOP("DynamicContext", "showInputMethodInformationWhenFocusIn"),
    "ActivateKeys": QT_TRANSLATE_NOOP("DynamicContext", "ActivateKeys"),
    "AltTriggerKeys": QT_TRANSLATE_NOOP("DynamicContext", "AltTriggerKeys"),
    "DeactivateKeys": QT_TRANSLATE_NOOP("DynamicContext", "DeactivateKeys"),
    "EnumerateBackwardKeys": QT_TRANSLATE_NOOP("DynamicContext", "Switch to the previous input method"),
    "EnumerateForwardKeys": QT_TRANSLATE_NOOP("DynamicContext", "Switch to the next input method"),
    "EnumerateGroupBackwardKeys": QT_TRANSLATE_NOOP("DynamicContext", "EnumerateGroupBackwardKeys"),
    "EnumerateSkipFirst": QT_TRANSLATE_NOOP("DynamicContext", "EnumerateSkipFirst"),
    "EnumerateGroupForwardKeys": QT_TRANSLATE_NOOP("DynamicContext", "EnumerateGroupForwardKeys"),
    "EnumerateWithTriggerKeys": QT_TRANSLATE_NOOP("DynamicContext", "EnumerateWithTriggerKeys"),
    "NextCandidate": QT_TRANSLATE_NOOP("DynamicContext", "NextCandidate"),
    "NextPage": QT_TRANSLATE_NOOP("DynamicContext", "NextPage"),
    "PrevCandidate": QT_TRANSLATE_NOOP("DynamicContext", "PrevCandidate"),
    "PrevPage": QT_TRANSLATE_NOOP("DynamicContext", "PrevPage"),
    "TogglePreedit": QT_TRANSLATE_NOOP("DynamicContext", "TogglePreedit"),
    "TriggerKeys": QT_TRANSLATE_NOOP("DynamicContext", "TriggerKeys"),
    "AutoSavePeriod": QT_TRANSLATE_NOOP("DynamicContext", "Frequency of automatic user data saving (minutes)"),
    "LongWordLengthLimit": QT_TRANSLATE_NOOP("DynamicContext", "LongWordLengthLimit")
};

function getTranslation(key) {
    return qsTranslate("DynamicContext", DynamicTranslations[key] || key);
}
