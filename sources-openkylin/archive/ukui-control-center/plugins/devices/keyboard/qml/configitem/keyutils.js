var KeyNames = (function (qt) {
  const KeyMappings = {
    // 修饰键
    [qt.Key_Tab]: "Tab",
    [qt.Key_Control]: "Ctrl",
    [qt.Key_Control_R]: "Ctrl",
    [qt.Key_Shift]: "Shift",
    [qt.Key_Shift_R]: "Shift",
    [qt.Key_Alt]: "Alt",
    [qt.Key_Alt_R]: "Alt",
    [qt.Key_AltGr]: "Alt",
    [qt.Key_Meta]: "Super",
    [qt.Key_Meta_L]: "Super",
    [qt.Key_Meta_R]: "Super",
    [qt.Key_Super]: "Super",
    [qt.Key_Super_L]: "Super",
    [qt.Key_Super_R]: "Super",

    // 特殊功能键
    [qt.Key_NumLock]: "NumLock",
    [qt.Key_ScrollLock]: "ScrollLock",
    [qt.Key_Print]: "PrintScreen",
    [qt.Key_Space]: "Space",
    [qt.Key_Enter]: "Enter",
    [qt.Key_Return]: "Enter",
    [qt.Key_Escape]: "Esc",
    [qt.Key_Backspace]: "Backspace",
    [qt.Key_Delete]: "Delete",
    [qt.Key_CapsLock]: "CapsLock",
    [qt.Key_Left]: "Left",
    [qt.Key_Up]: "Up",
    [qt.Key_Right]: "Right",
    [qt.Key_Down]: "Down",
    [qt.Key_Insert]: "Insert",
    [qt.Key_Home]: "Home",
    [qt.Key_End]: "End",
    [qt.Key_Pause]: "Pause",
    [qt.Key_PageUp]: "PageUp",
    [qt.Key_PageDown]: "PageDown",

    [qt.Key_F1]: "F1",
    [qt.Key_F2]: "F2",
    [qt.Key_F3]: "F3",
    [qt.Key_F4]: "F4",
    [qt.Key_F5]: "F5",
    [qt.Key_F6]: "F6",
    [qt.Key_F7]: "F7",
    [qt.Key_F8]: "F8",
    [qt.Key_F9]: "F9",
    [qt.Key_F10]: "F10",
    [qt.Key_F11]: "F11",
    [qt.Key_F12]: "F12",
    [qt.Key_F13]: "F13",
  };

  return {
    mappings: KeyMappings,
    getName: function (key) {
      const numKey = Number(key);
      return this.mappings[numKey] || String.fromCharCode(numKey);
    },
  };
})(Qt);

var KeyPadNames = (function (qt) {
  const KeyMappings = {
    // 数字键（小键盘区）
    [qt.Key_0]: "Num_0",
    [qt.Key_1]: "Num_1",
    [qt.Key_2]: "Num_2",
    [qt.Key_3]: "Num_3",
    [qt.Key_4]: "Num_4",
    [qt.Key_5]: "Num_5",
    [qt.Key_6]: "Num_6",
    [qt.Key_7]: "Num_7",
    [qt.Key_8]: "Num_8",
    [qt.Key_9]: "Num_9",

    [qt.Key_Asterisk]: "Num_*",
    [qt.Key_Plus]: "Num_+",
    [qt.Key_Minus]: "Num_-",
    [qt.Key_Period]: "Num_.",
    [qt.Key_Slash]: "Num_/",
    [qt.Key_Enter]: "Num_Enter",

    // 小键盘区（NumLock关闭时）
    [qt.Key_Begin]: "Num_Begin",
    [qt.Key_Home]: "Num_Home",
    [qt.Key_Up]: "Num_Up",
    [qt.Key_PageUp]: "Num_PageUp",
    [qt.Key_Prior]: "Num_Prior",
    [qt.Key_Left]: "Num_Left",
    [qt.Key_Clear]: "Num_Clear",
    [qt.Key_Right]: "Num_Right",
    [qt.Key_End]: "Num_End",
    [qt.Key_Down]: "Num_Down",
    [qt.Key_PageDown]: "Num_PageDown",
    [qt.Key_Insert]: "Num_Insert",
    [qt.Key_Delete]: "Num_Delete",
  };

  return {
    mappings: KeyMappings,
    getName: function (key) {
      const numKey = Number(key);
      return this.mappings[numKey] || String.fromCharCode(numKey);
    },
  };
})(Qt);

function getKeyName(key) {
  return KeyNames.getName(key);
}

function getKeyPadName(key) {
  return KeyPadNames.getName(key);
}

const ModifierMap = {
  Ctrl_L: "Ctrl",
  Ctrl_R: "Ctrl",
  Shift_L: "Shift",
  Shift_R: "Shift",
  Alt_L: "Alt",
  Alt_R: "Alt",
  Meta_L: "Meta",
  Meta_R: "Meta",
  Super_L: "Super",
  Super_R: "Super",
};

function beautifyModifierKey(rawKeyName) {
  return ModifierMap[rawKeyName] || rawKeyName;
}

function sortAndDeduplicateKeys(keys) {
  const showKeys = keys.split(" ").map((key) => {
    const beautified = beautifyModifierKey(key.trim());
    console.log("beautified:", beautified);
    return beautified || key.trim();
  });

  const countMap = {};
  showKeys.forEach((key) => {
    countMap[key] = (countMap[key] || 0) + 1;
  });

  const filtered = showKeys.filter((item, index) => {
    return showKeys.indexOf(item) === index;
  });

  const sortedKeys = filtered.sort((a, b) => {
    return countMap[a] - countMap[b];
  });

  return sortedKeys.join(" ");
}

const scanCodeMap = {
  49: "~",
  50: "Shift_L",
  62: "Shift_R",
  37: "Ctrl_L",
  105: "Ctrl_R",
  64: "Alt_L",
  108: "Alt_R",
  10: "!",
  11: "@",
  12: "#",
  13: "$",
  14: "%",
  15: "^",
  16: "&",
  17: "*",
  18: "(",
  19: ")",
  20: "_",
  21: "+",
  34: "{",
  35: "}",
  47: ":",
  48: '"',
  51: "|",
  59: "<",
  60: ">",
  61: "?",
  23: "Tab",
};

const scanCodeKeyPad = [
  87, 88, 89, 83, 84, 85, 79, 80, 81, 106, 63, 82, 86, 91, 104, 90,
];

const KeyMaps = {
  scanCode: scanCodeMap,
};

const mainKbDigitMofified = [
  "~",
  "!",
  "@",
  "#",
  "$",
  "%",
  "^",
  "&",
  "*",
  "(",
  ")",
  "{",
  "}",
  ":",
  '"',
  "|",
  "<",
  ">",
  "?",
  "+",
  "_",
];
const UnifiedKeyMap = {
  _mappings: {
    // 修饰键
    Control: { qml: "Ctrl", qt: Qt.Key_Control },
    Control_R: { qml: "Ctrl_R", qt: Qt.Key_Control_R },
    Control_L: { qml: "Ctrl_L", qt: Qt.Key_Control },
    Shift: { qml: "Shift", qt: Qt.Key_Shift },
    Shift_L: { qml: "Shift_L", qt: Qt.Key_Shift },
    Shift_R: { qml: "Shift_R", qt: Qt.Key_Shift_R },
    Alt: { qml: "Alt", qt: Qt.Key_Alt },
    Alt_L: { qml: "Alt_L", qt: Qt.Key_Alt },
    Alt_R: { qml: "Alt_R", qt: Qt.Key_Alt_R },
    Meta: { qml: "Meta", qt: Qt.Key_Meta },
    Meta_L: { qml: "Meta_L", qt: Qt.Key_Meta },
    Meta_R: { qml: "Meta_R", qt: Qt.Key_Meta_R },
    Super: { qml: "Super", qt: Qt.Key_Super },
    Super_L: { qml: "Super_L", qt: Qt.Key_Super },
    Super_R: { qml: "Super_R", qt: Qt.Key_Super },

    // 功能键
    space: { qml: "Space", qt: Qt.Key_Space },
    Escape: { qml: "Esc", qt: Qt.Key_Escape },
    Return: { qml: "Enter", qt: Qt.Key_Return },
    Backspace: { qml: "Backspace", qt: Qt.Key_Backspace },
    Num_Lock: { qml: "NumLock", qt: Qt.Key_NumLock },
    Caps_Lock: { qml: "CapsLock", qt: Qt.Key_CapsLock },
    Delete: { qml: "Delete", qt: Qt.Key_Delete },
    Scroll_Lock: { qml: "ScrollLock", qt: Qt.Key_ScrollLock },
    Print: { qml: "PrintScreen", qt: Qt.Key_Print },
    Left: { qml: "Left", qt: Qt.Key_Left },
    Right: { qml: "Right", qt: Qt.Key_Right },
    Down: { qml: "Down", qt: Qt.Key_Down },
    Up: { qml: "Up", qt: Qt.Key_Up },

    Insert: { qml: "Insert", qt: Qt.Key_Insert },
    Home: { qml: "Home", qt: Qt.Key_Home },
    End: { qml: "End", qt: Qt.Key_End },

    Page_Up: { qml: "PageUp", qt: Qt.Key_PageUp },
    Next: { qml: "PageDown", qt: Qt.Key_PageDown },

    Pause: { qml: "Pause", qt: Qt.Key_Pause },

    // 小键盘数字
    KP_0: { qml: "Num_0", qt: Qt.Key_Num0 },
    KP_1: { qml: "Num_1", qt: Qt.Key_Num1 },
    KP_2: { qml: "Num_2", qt: Qt.Key_Num2 },
    KP_3: { qml: "Num_3", qt: Qt.Key_Num3 },
    KP_4: { qml: "Num_4", qt: Qt.Key_Num4 },
    KP_5: { qml: "Num_5", qt: Qt.Key_Num5 },
    KP_6: { qml: "Num_6", qt: Qt.Key_Num6 },
    KP_7: { qml: "Num_7", qt: Qt.Key_Num7 },
    KP_8: { qml: "Num_8", qt: Qt.Key_Num8 },
    KP_9: { qml: "Num_9", qt: Qt.Key_Num9 },

    // 小键盘运算符
    KP_Divide: { qml: "Num_/", qt: Qt.Key_Divide },
    KP_Add: { qml: "Num_+", qt: Qt.Key_Add },
    KP_Subtract: { qml: "Num_-", qt: Qt.Key_Subtract },
    KP_Multiply: { qml: "Num_*", qt: Qt.Key_Multiply },
    KP_Decimal: { qml: "Num_.", qt: Qt.Key_Decimal },

    // 小键盘功能键
    KP_Begin: { qml: "Num_Begin", qt: Qt.Key_Begin },
    KP_Delete: { qml: "Num_Delete", qt: Qt.Key_Delete },
    KP_Up: { qml: "Num_Up", qt: Qt.Key_Down },
    KP_Down: { qml: "Num_Down", qt: Qt.Key_Down },
    KP_End: { qml: "Num_End", qt: Qt.Key_End },
    KP_Enter: { qml: "Num_Enter", qt: Qt.Key_Enter },
    KP_Home: { qml: "Num_Home", qt: Qt.Key_Home },
    KP_Insert: { qml: "Num_Insert", qt: Qt.Key_Insert },
    KP_Left: { qml: "Num_Left", qt: Qt.Key_Left },
    KP_Next: { qml: "Num_PageDown", qt: Qt.Key_PageDown },
    KP_Page_Up: { qml: "Num_PageUp", qt: Qt.Key_PageUp },
    KP_Right: { qml: "Num_Right", qt: Qt.Key_Right },

    // 符号键（无Qt枚举值）
    asciitilde: { qml: "~", qt: null },
    grave: { qml: "`", qt: null },
    exclam: { qml: "!", qt: null },
    at: { qml: "@", qt: null },
    numbersign: { qml: "#", qt: null },
    dollar: { qml: "$", qt: null },
    percent: { qml: "%", qt: null },
    asciicircum: { qml: "^", qt: null },
    ampersand: { qml: "&", qt: null },
    asterisk: { qml: "*", qt: null },
    parenleft: { qml: "(", qt: null },
    parenright: { qml: ")", qt: null },
    underscore: { qml: "_", qt: null },
    minus: { qml: "-", qt: Qt.Key_Minus },
    plus: { qml: "+", qt: null },
    equal: { qml: "=", qt: null },
    bracketleft: { qml: "[", qt: null },
    braceleft: { qml: "{", qt: null },
    bracketright: { qml: "]", qt: null },
    braceright: { qml: "}", qt: null },
    backslash: { qml: "\\", qt: null },
    bar: { qml: "|", qt: null },
    semicolon: { qml: ";", qt: null },
    colon: { qml: ":", qt: null },
    apostrophe: { qml: "'", qt: null },
    quotedbl: { qml: '"', qt: null },
    comma: { qml: ",", qt: null },
    less: { qml: "<", qt: null },
    period: { qml: ".", qt: null },
    greater: { qml: ">", qt: null },
    slash: { qml: "/", qt: null },
    question: { qml: "?", qt: null },
  },

  // 初始化反向索引
  _init: function () {
    this._qmlToCpp = {};
    this._qmlToQt = {};

    for (const [cppKey, data] of Object.entries(this._mappings)) {
      if (!this._qmlToCpp[data.qml]) {
        this._qmlToCpp[data.qml] = cppKey;
      }
      if (data.qt !== null && !this._qmlToQt[data.qml]) {
        this._qmlToQt[data.qml] = data.qt;
      }
    }
  },

  cppToQml: function (cppKey) {
    return (this._mappings[cppKey] && this._mappings[cppKey].qml) || cppKey;
  },

  qmlToCpp: function (qmlKey) {
    return this._qmlToCpp[qmlKey] || qmlKey;
  },

  cppToQt: function (cppKey) {
    return (this._mappings[cppKey] && this._mappings[cppKey].qt) || null;
  },

  qmlToQt: function (qmlKey) {
    return this._qmlToQt[qmlKey] || null;
  },
};

UnifiedKeyMap._init();

function sortModifiers(keys) {
  const priorityOrder = [Qt.Key_Control, Qt.Key_Alt, Qt.Key_Shift, Qt.Key_Meta];

  return keys.sort((a, b) => {
    const indexA = priorityOrder.indexOf(a.key);
    const indexB = priorityOrder.indexOf(b.key);

    if (indexA === -1) {
      return 1;
    }
    if (indexB === -1) {
      return -1;
    }
    return indexA - indexB;
  });
}

function processCombinationKeys(currentKeys) {
  if (currentKeys.length === 0) {
    return "";
  }
  if (areAllModifiers(currentKeys)) {
    return handleAllModifiers(currentKeys);
  }
  if (currentKeys.length === 1) {
    return handleSingleKey(currentKeys[0]);
  }
  return getKeysText(currentKeys);
}

function areAllModifiers(currentKeys) {
  return currentKeys.every((data) => data.isModifier === true);
}

function handleAllModifiers(currentKeys) {
  if (currentKeys.length === 1) {
    return KeyMaps.scanCode[currentKeys[0].code] || "";
  }

  const modifiedKeys = [...currentKeys];
  const lastKey = modifiedKeys.pop();

  let lastKeyStr = "";
  if (lastKey) {
    lastKeyStr = KeyMaps.scanCode[lastKey.code] || "";
  }

  const sortedKeys = sortModifiers(modifiedKeys);
  let sortedKeysText = getKeysText(sortedKeys);

  if (sortedKeysText && lastKeyStr) {
    sortedKeysText += ` ${lastKeyStr}`;
  }

  return sortedKeysText;
}

function handleSingleKey(keyData) {
  if (keyData.needModify) {
    const modifiedKey = KeyMaps.scanCode[keyData.code] || "";
    return modifiedKey === "" ? getKeyName(keyData.key) : modifiedKey;
  }
  if (keyData.isKeyPadKey) {
    return getKeyPadName(keyData.key);
  }
  return getKeyName(keyData.key);
}

function getKeysText(currentKeys) {
  const keys = currentKeys.map((keyData) => {
    if (keyData.needModify) {
      const modifiedKey = KeyMaps.scanCode[keyData.code] || "";
      return modifiedKey === "" ? getKeyName(keyData.key) : modifiedKey;
    }
    if (keyData.isKeyPadKey) {
      return getKeyPadName(keyData.key);
    }
    return getKeyName(keyData.key);
  });
  return keys.join(" ");
}
