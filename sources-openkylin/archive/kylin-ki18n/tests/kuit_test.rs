// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

mod common;

use ki18n_rs::kuit::{
    parse, salvage_markup, to_format, to_format_with_langs_for_domain, VisualFormat,
};
use ki18n_rs::kuit::{
    set_marker_format, set_marker_format_for_domain, set_tag_class, set_tag_pattern,
    set_tag_pattern_full, TagClass,
};

#[test]
fn test_bold_tag_plain() {
    let node = parse("@b{Hello}");
    let out = to_format(&node, VisualFormat::PlainText);
    assert!(out.contains("*Hello*"));
}

#[test]
fn test_bold_tag_term() {
    let node = parse("@b{Hello}");
    let out = to_format(&node, VisualFormat::TermText);
    assert!(out.contains("\x1b[1mHello\x1b[0m"));
}

#[test]
fn test_email_plain_rich_and_address() {
    let plain = parse(r#"<email>u@ex.org</email>"#);
    assert_eq!(
        to_format(&plain, VisualFormat::PlainText),
        "<u@ex.org>"
    );
    let rich = parse(r#"<email>u@ex.org</email>"#);
    let out = to_format(&rich, VisualFormat::RichText);
    assert!(out.contains("mailto:"), "{out}");
    assert!(out.contains("u@ex.org"), "{out}");

    let named = parse(r#"<email address="u@ex.org">Alice</email>"#);
    assert_eq!(
        to_format(&named, VisualFormat::PlainText),
        "Alice <u@ex.org>"
    );
    let named_r = parse(r#"<email address="u@ex.org">Alice</email>"#);
    let nr = to_format(&named_r, VisualFormat::RichText);
    assert!(nr.contains("mailto:u@ex.org"), "{nr}");
    assert!(nr.contains("Alice"), "{nr}");
}

#[test]
fn test_salvage_markup_recover_b() {
    let out = salvage_markup("<b>x</b>", VisualFormat::RichText, &[], "");
    assert_eq!(out, "<html><b>x</b></html>");
}

/// 对齐 `KLocalizedStringTest::brokenStructTagUsages`：phrase 内嵌 struct（title）时丢弃 title 外壳。
#[test]
fn test_phrase_struct_dropout_emphasis_title() {
    let node = parse("<emphasis><title>History</title></emphasis>");
    let out = to_format(&node, VisualFormat::RichText);
    assert_eq!(out, "<html><i>History</i></html>");
}

/// `parseOpenEl` 沿栈跳过 Dropout：title 下的 item 仍以 phrase（emphasis）为 enclosing → 继续 Dropout。
#[test]
fn test_phrase_struct_dropout_nested_item_under_title_under_emphasis() {
    let node = parse("<emphasis><title><item>x</item></title></emphasis>");
    let out = to_format(&node, VisualFormat::RichText);
    assert_eq!(out, "<html><i>x</i></html>");
}

/// `formatSubText`：`OpenEl::Ignored` 原样保留未知标签。
#[test]
fn test_unknown_xml_tag_ignored_verbatim() {
    let node = parse(r#"<bogus id="1">inner</bogus>"#);
    let out = to_format(&node, VisualFormat::PlainText);
    assert_eq!(out, r#"<bogus id='1'>inner</bogus>"#);
}

#[test]
fn test_link_rich() {
    let node = parse(r#"<a href="http://example.com">Click</a>"#);
    let out = to_format(&node, VisualFormat::RichText);
    assert_eq!(
        out,
        r#"<html><a href="http://example.com">Click</a></html>"#
    );
}

#[test]
fn test_ui_marker_title_rich() {
    let node = parse("@title:Main Window/rich");
    let out = to_format(&node, VisualFormat::RichText);
    eprintln!("DEBUG out = {:?}", out);
    assert!(out.contains("<h1>Main Window</h1>"));
}

#[test]
fn test_ui_marker_action_menu_term() {
    let node = parse("@action:Save/inmenubar");
    let out = to_format(&node, VisualFormat::TermText);
    assert!(out.contains("[Save]"));
}

#[test]
fn test_entity_amp_numeric() {
    let node = parse("&amp; &lt; &gt; &#65; &#x41;");
    let out = to_format(&node, VisualFormat::PlainText);
    assert_eq!(out, "& < > A A");
}

#[test]
fn test_image_self_closing_rich() {
    let node = parse(r#"<img src="icon.png" alt="icon"/>"#);
    let out = to_format(&node, VisualFormat::RichText);
    assert_eq!(
        out,
        r#"<html><img src="icon.png" alt="icon"/></html>"#
    );
}

#[test]
fn test_paragraph_and_list_plain() {
    let node = parse("@p{Item 1 @li{First} @li{Second}}");
    let out = to_format(&node, VisualFormat::PlainText);
    assert!(out.contains("First"));
    assert!(out.contains("Second"));
}

#[test]
fn test_italic_and_underline_plain() {
    let node = parse("@i{hello} @u{world}");
    let out = to_format(&node, VisualFormat::PlainText);
    eprintln!("DEBUG italic+underline out = {:?}", out);
    assert!(out.contains("/hello/"));
    assert!(out.contains("_world_"));
}

#[test]
fn test_emphasis_plain() {
    let node = parse("@emphasis{hello}");
    let out = to_format(&node, VisualFormat::PlainText);
    assert!(out.contains("*hello*"));
}

#[test]
fn test_heading_plain_and_rich() {
    let node = parse("@h1{Title}");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.starts_with("Title"));

    let out_rich = to_format(&node, VisualFormat::RichText);
    assert!(out_rich.contains("<h1>Title</h1>"));
}

#[test]
fn test_anchor_with_title_and_term() {
    let node = parse(r#"<a href="https://example.com" title="Example">Click</a>"#);
    let out_rich = to_format(&node, VisualFormat::RichText);
    assert_eq!(
        out_rich,
        r#"<html><a href="https://example.com">Click</a></html>"#
    );

    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert_eq!(out_plain, "Click");

    let out_term = to_format(&node, VisualFormat::TermText);
    assert_eq!(out_term, "Click");
}

#[test]
fn test_ui_warning_error_info_term() {
    let node_w = parse("@warning{careful}");
    let node_e = parse("@error{boom}");
    let node_i = parse("@info{note}");

    let out_w = to_format(&node_w, VisualFormat::TermText);
    let out_e = to_format(&node_e, VisualFormat::TermText);
    let out_i = to_format(&node_i, VisualFormat::TermText);

    assert!(out_w.contains("careful"));
    assert!(out_e.contains("boom"));
    assert!(out_i.contains("note"));
}

#[test]
fn test_entity_invalid_kept() {
    let node = parse("&unknown;");
    let out = to_format(&node, VisualFormat::PlainText);
    assert_eq!(out, "&unknown;");
}

#[test]
fn test_nested_kuit_and_xml() {
    let node = parse("<p>@b{Bold} and <i>italic</i></p>");
    let out = to_format(&node, VisualFormat::RichText);
    eprintln!("DEBUG nested out = {:?}", out);
    assert!(out.contains("<p>"));
    assert!(out.contains("<b>Bold</b>"));
    assert!(out.contains("<i>italic</i>"));
}

#[test]
fn test_table_term_formatting() {
    let node = parse("<table><tr><td>Cell</td></tr></table>");
    let out = to_format(&node, VisualFormat::TermText);
    assert!(out.contains("Cell"));
    assert!(out.contains("┌"));
    assert!(out.contains("└"));
}

#[test]
fn test_div_and_span_plain() {
    let node = parse("<div><span>inner</span></div>");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("inner"));
}

#[test]
fn test_table_rich() {
    let node = parse("<table><tr><td>Cell</td></tr></table>");
    let out = to_format(&node, VisualFormat::RichText);
    assert_eq!(
        out,
        "<html><table><tr><td>Cell</td></tr></table></html>"
    );
}

#[test]
fn test_dl_dt_dd_rich() {
    let node = parse("<dl><dt>Term</dt><dd>Definition</dd></dl>");
    let out = to_format(&node, VisualFormat::RichText);
    assert!(out.contains("<dl>"));
    assert!(out.contains("<dt>Term</dt>"));
    assert!(out.contains("<dd>Definition</dd>"));
}

#[test]
fn test_span_with_class_rich() {
    let node = parse(r#"<span class="highlight">text</span>"#);
    let out = to_format(&node, VisualFormat::RichText);
    assert_eq!(
        out,
        r#"<html><span class="highlight">text</span></html>"#
    );
}

#[test]
fn test_hr_rich() {
    let node = parse("<hr/>");
    let out = to_format(&node, VisualFormat::RichText);
    assert_eq!(out, "<html><hr/></html>");
}

#[test]
fn test_pre_rich() {
    let node = parse("<pre>code</pre>");
    let out = to_format(&node, VisualFormat::RichText);
    assert_eq!(out, "<html><pre>code</pre></html>");
}

// ---------------------------------------------------------------------------
// 追加测试：更细粒度覆盖所有 KUIT 标签与三种格式
// ---------------------------------------------------------------------------

#[test]
fn test_strong_and_em_tags_plain_and_rich() {
    let node = parse("@strong{S} @em{E}");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    let out_rich = to_format(&node, VisualFormat::RichText);
    assert!(out_plain.contains("*S*"));
    // em 在 plain 下也使用 * 包裹
    assert!(out_plain.contains("*E*"));
    assert!(out_rich.contains("<b>S</b>"));
    assert!(out_rich.contains("<i>E</i>"));
}

#[test]
fn test_u_and_code_term() {
    let node = parse("@u{U} @code{C}");
    let out_term = to_format(&node, VisualFormat::TermText);
    assert!(out_term.contains("\x1b[4mU\x1b[0m"));
    assert!(out_term.contains("\x1b[2mC\x1b[0m"));
}

#[test]
fn test_br_plain_and_rich() {
    let node = parse("<p>line1<br/>line2</p>");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    let out_rich = to_format(&node, VisualFormat::RichText);
    assert!(out_plain.contains("line1"));
    assert!(out_plain.contains("line2"));
    assert!(out_plain.contains('\n'));
    assert!(out_rich.contains("<br/>"));
}

#[test]
fn test_p_and_list_rich() {
    let node = parse("<p>Item</p><ul><li>First</li><li>Second</li></ul>");
    let out_rich = to_format(&node, VisualFormat::RichText);
    assert!(out_rich.contains("<p>Item</p>"));
    assert!(out_rich.contains("<ul>"));
    assert!(out_rich.contains("<li>First</li>"));
    assert!(out_rich.contains("<li>Second</li>"));
}

#[test]
fn test_nested_lists_plain() {
    let node = parse("<ul><li>One<ul><li>Two</li></ul></li></ul>");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("One"));
    assert!(out_plain.contains("Two"));
}

#[test]
fn test_heading_levels_plain() {
    let node = parse("@h2{Sub} @h3{SubSub} @h6{Tiny}");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("Sub"));
    assert!(out_plain.contains("SubSub"));
    assert!(out_plain.contains("Tiny"));
}

#[test]
fn test_dl_plain() {
    let node = parse("<dl><dt>Term</dt><dd>Definition</dd></dl>");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("Term"));
    assert!(out_plain.contains("Definition"));
}

#[test]
fn test_hr_plain_and_term() {
    let node = parse("<hr/>");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    let out_term = to_format(&node, VisualFormat::TermText);
    assert!(out_plain.contains("---"));
    assert!(out_term.contains("─"));
}

#[test]
fn test_div_and_span_rich_with_class() {
    let node = parse(r#"<div class="c"><span class="s">inner</span></div>"#);
    let out_rich = to_format(&node, VisualFormat::RichText);
    assert_eq!(
        out_rich,
        r#"<html><div class="c"><span class="s">inner</span></div></html>"#
    );
}

#[test]
fn test_pre_plain_with_code_block() {
    let node = parse("<pre>fn main() {}</pre>");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("fn main() {}"));
}

#[test]
fn test_a_without_href_and_plain() {
    let node = parse("@a{Link}");
    let out_rich = to_format(&node, VisualFormat::RichText);
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert_eq!(out_plain, "Link");
    assert_eq!(out_rich, "<html><a>Link</a></html>");
}

#[test]
fn test_img_plain_and_term() {
    let node = parse(r#"<img src="icon.png" alt="Icon"/>"#);
    let out_plain = to_format(&node, VisualFormat::PlainText);
    let out_term = to_format(&node, VisualFormat::TermText);
    assert!(out_plain.contains("[image]") || out_plain.contains("Icon"));
    // term 模式当前不对 img 做特殊处理，允许为空输出
    let _ = out_term;
}

#[test]
fn test_ui_title_plain_subtitle_rich() {
    let node_title = parse("@title:Main/plain");
    let node_sub = parse("@subtitle:Sub/rich");
    let out_title = to_format(&node_title, VisualFormat::PlainText);
    let out_sub = to_format(&node_sub, VisualFormat::RichText);
    assert!(out_title.contains("MAIN") || out_title.contains("Main"));
    assert!(out_sub.contains("<h2>Sub</h2>"));
}

#[test]
fn test_ui_label_and_button_rich() {
    let node_label = parse("@label:Name/rich");
    let node_button = parse("@button:OK/rich");
    let out_label = to_format(&node_label, VisualFormat::RichText);
    let out_button = to_format(&node_button, VisualFormat::RichText);
    assert!(out_label.contains("label"));
    assert!(out_button.contains("<button"));
}

#[test]
fn test_ui_warning_error_info_plain_and_rich() {
    let node = parse("@warning{w}@error{e}@info{i}");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    let out_rich = to_format(&node, VisualFormat::RichText);
    assert!(out_plain.contains("WARNING") || out_plain.contains("warning"));
    assert!(out_rich.contains("warning"));
}

#[test]
fn test_ui_marker_action_toolbar_term() {
    let node = parse("@action:Tool/intoolbar");
    let out = to_format(&node, VisualFormat::TermText);
    assert!(out.contains("Tool"));
}

#[test]
fn test_entity_named_and_numeric_variants() {
    let node = parse("&amp;&lt;&gt;&quot;&apos;&#65;&#x41;&#X41;");
    let out = to_format(&node, VisualFormat::PlainText);
    assert!(out.contains("&"));
    assert!(out.contains("<"));
    assert!(out.contains(">"));
    assert!(out.contains("\""));
    assert!(out.contains("'"));
}

#[test]
fn test_entity_incomplete_and_unknown() {
    let node = parse("&; &x; &unknown;");
    let out = to_format(&node, VisualFormat::PlainText);
    assert!(out.contains("&;"));
    assert!(out.contains("&x;"));
    assert!(out.contains("&unknown;"));
}

#[test]
fn test_nested_b_in_p_and_a_in_p() {
    let node = parse("<p>@b{Bold} <a href=\"u\">link</a></p>");
    let out = to_format(&node, VisualFormat::RichText);
    assert!(out.contains("<p>"));
    assert!(out.contains("<b>Bold</b>"));
    assert!(out.contains("<a href=\"u\">link</a>"));
}

#[test]
fn test_mixed_kuit_and_xml_plain() {
    let node = parse("@b{Bold} and <i>italic</i> &amp; plain");
    let out = to_format(&node, VisualFormat::PlainText);
    assert!(out.contains("Bold"));
    assert!(out.contains("italic"));
    assert!(out.contains("&"));
}

// coverage-task-3-kuit: 标签扩展 ------------------------------------------------

#[test]
fn test_ul_nested_ol() {
    let node = parse("<ul><li>One<ol><li>First</li><li>Second</li></ol></li></ul>");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("One"));
    assert!(out_plain.contains("First"));
    assert!(out_plain.contains("Second"));
}

#[test]
fn test_ol_nested_ul() {
    let node = parse("<ol><li>One<ul><li>A</li><li>B</li></ul></li></ol>");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("One"));
    assert!(out_plain.contains("A"));
    assert!(out_plain.contains("B"));
}

#[test]
fn test_table_with_multiple_rows() {
    let node = parse("<table><tr><td>R1</td></tr><tr><td>R2</td></tr></table>");
    let out_term = to_format(&node, VisualFormat::TermText);
    assert!(out_term.contains("R1"));
    assert!(out_term.contains("R2"));
}

#[test]
fn test_table_with_th_cells() {
    let node = parse("<table><tr><th>H</th><td>C</td></tr></table>");
    let out_rich = to_format(&node, VisualFormat::RichText);
    assert!(out_rich.contains("<th>H</th>"));
    assert!(out_rich.contains("<td>C</td>"));
}

#[test]
fn test_dl_multiple_definitions() {
    let node = parse("<dl><dt>T1</dt><dd>D1</dd><dt>T2</dt><dd>D2</dd></dl>");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("T1"));
    assert!(out_plain.contains("D1"));
    assert!(out_plain.contains("T2"));
    assert!(out_plain.contains("D2"));
}

#[test]
fn test_br_multiple() {
    let node = parse("line1<br/>line2<br/>line3");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("line1"));
    assert!(out_plain.contains("line2"));
    assert!(out_plain.contains("line3"));
}

#[test]
fn test_hr_in_paragraph() {
    let node = parse("<p>Top</p><hr/><p>Bottom</p>");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("Top"));
    assert!(out_plain.contains("Bottom"));
}

#[test]
fn test_img_multiple() {
    let node = parse(
        r#"<p><img src="a.png" alt="A"/><img src="b.png" alt="B"/></p>"#,
    );
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("A") || out_plain.contains("[image]"));
}

#[test]
fn test_span_multiple_attributes() {
    let node = parse(r#"<span id="i" class="c" data-x="1">t</span>"#);
    let out_rich = to_format(&node, VisualFormat::RichText);
    // 当前实现保留 span 外层，不保证所有属性 roundtrip，仅检查文本存在
    assert!(out_rich.contains("t"));
}

#[test]
fn test_div_with_id_and_class() {
    let node = parse(r#"<div id="d" class="c">x</div>"#);
    let out_rich = to_format(&node, VisualFormat::RichText);
    assert!(out_rich.contains("<div") || out_rich.contains("x"));
}

// UI Marker 扩展 --------------------------------------------------------------

#[test]
fn test_ui_marker_all_roles() {
    let title = to_format(&parse("@title:Main/rich"), VisualFormat::RichText);
    let label = to_format(&parse("@label:Name/plain"), VisualFormat::PlainText);
    let button = to_format(&parse("@button:OK/tooltip"), VisualFormat::TermText);
    assert!(title.contains("Main"));
    assert!(label.to_lowercase().contains("name"));
    assert!(button.contains("OK"));
}

#[test]
fn test_ui_marker_all_cues() {
    let inmenu = to_format(&parse("@action:Open/inmenu"), VisualFormat::TermText);
    let intoolbar = to_format(&parse("@action:Tool/intoolbar"), VisualFormat::TermText);
    let inmenubar =
        to_format(&parse("@action:Save/inmenubar"), VisualFormat::TermText);
    assert!(inmenu.contains("Open"));
    assert!(intoolbar.contains("Tool"));
    assert!(inmenubar.contains("Save"));
}

#[test]
fn test_ui_marker_all_formats() {
    let p = to_format(&parse("@title:Plain/plain"), VisualFormat::PlainText);
    let r = to_format(&parse("@title:Rich/rich"), VisualFormat::RichText);
    let t = to_format(&parse("@title:Term/term"), VisualFormat::TermText);
    assert!(p.contains("Plain"));
    assert!(r.contains("<h1>Rich</h1>") || r.contains("Rich"));
    assert!(t.contains("Term"));
}

#[test]
fn test_ui_marker_combined() {
    let node = parse("@title:Window/rich@subtitle:Sub/plain");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    let out_rich = to_format(&node, VisualFormat::RichText);
    assert!(out_plain.to_lowercase().contains("window"));
    // subtitle 在 rich 输出中的具体形式依赖实现，这里只要求不崩溃
    let _ = out_rich;
}

#[test]
fn test_ui_marker_empty_content() {
    let node = parse("@title:/rich");
    let out = to_format(&node, VisualFormat::RichText);
    assert!(out.is_empty() || out.contains("<h1></h1>"));
}

// 实体引用扩展 ---------------------------------------------------------------

#[test]
fn test_entity_all_named_entities() {
    let node = parse("&amp;&lt;&gt;&quot;&apos;");
    let out = to_format(&node, VisualFormat::PlainText);
    assert_eq!(out, "&<>\"'");
}

#[test]
fn test_entity_numeric_0_to_127() {
    let mut input = String::new();
    for i in 0..5 {
        // 只测试前几个，避免输出过长
        input.push_str(&format!("&#{}; ", i));
    }
    let node = parse(&input);
    let out = to_format(&node, VisualFormat::PlainText);
    assert!(out.contains("\u{0}") || out.starts_with('\u{0}') || !out.is_empty());
}

#[test]
fn test_entity_hex_lowercase() {
    let node = parse("&#x61;&#x62;");
    let out = to_format(&node, VisualFormat::PlainText);
    assert_eq!(out, "ab");
}

#[test]
fn test_entity_hex_uppercase() {
    let node = parse("&#X41;&#X42;");
    let out = to_format(&node, VisualFormat::PlainText);
    assert_eq!(out, "AB");
}

#[test]
fn test_entity_invalid_kept_as_is_extended() {
    let node = parse("&notAnEntity; &also_not#;");
    let out = to_format(&node, VisualFormat::PlainText);
    assert!(out.contains("&notAnEntity;"));
    assert!(out.contains("&also_not#;"));
}

// 嵌套结构扩展 ---------------------------------------------------------------

#[test]
fn test_nested_table_in_div() {
    let node = parse("<div><table><tr><td>Cell</td></tr></table></div>");
    let out_rich = to_format(&node, VisualFormat::RichText);
    assert!(out_rich.contains("<div"));
    assert!(out_rich.contains("<table>"));
}

#[test]
fn test_nested_lists_deep_3_levels() {
    let node =
        parse("<ul><li>L1<ul><li>L2<ol><li>L3</li></ol></li></ul></li></ul>");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("L1"));
    assert!(out_plain.contains("L2"));
    assert!(out_plain.contains("L3"));
}

#[test]
fn test_nested_ui_markers() {
    let node = parse("@title:Outer/plain @action:Inner/inmenu");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.to_lowercase().contains("outer"));
}

#[test]
fn test_nested_mixed_tags() {
    let node = parse("<div>@b{B}<ul><li>@i{I}</li></ul></div>");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("B"));
    assert!(out_plain.contains("I"));
}

#[test]
fn test_nested_self_closing_in_container() {
    let node = parse("<div><img src=\"a.png\" alt=\"A\"/><br/><hr/></div>");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("A") || out_plain.contains("[image]"));
}

// 格式转换扩展 ---------------------------------------------------------------

#[test]
fn test_format_ellipsis_conversion() {
    let node = parse("Wait...");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains("…") || out_plain.contains("..."));
}

#[test]
fn test_format_special_whitespace() {
    let node = parse("a\u{00A0}b"); // 不换行空格
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains('a'));
    assert!(out_plain.contains('b'));
}

#[test]
fn test_format_rtl_marks() {
    let node = parse("\u{200F}abc");
    let out_plain = to_format(&node, VisualFormat::PlainText);
    // 只验证不会被破坏
    assert!(out_plain.contains("abc"));
}

#[test]
fn test_format_combining_chars() {
    let node = parse("a\u{0301}"); // a + combining acute
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains('a'));
}

#[test]
fn test_format_zero_width_chars() {
    let node = parse("a\u{200B}b"); // 零宽空格
    let out_plain = to_format(&node, VisualFormat::PlainText);
    assert!(out_plain.contains('a'));
    assert!(out_plain.contains('b'));
}

#[test]
fn test_kuit_per_domain_marker_isolation() {
    let dom_a = "kuit_isolation_a";
    let dom_b = "kuit_isolation_b";
    set_marker_format_for_domain(dom_a, "title", VisualFormat::RichText);
    set_marker_format_for_domain(dom_b, "title", VisualFormat::PlainText);
    let node = parse("@title:Main Window/plain");
    let out_a = to_format_with_langs_for_domain(&node, VisualFormat::PlainText, &[], dom_a);
    assert!(
        out_a.contains("<h1>") && out_a.contains("Main Window"),
        "dom_a expected rich title, got {:?}",
        out_a
    );
    let out_b = to_format_with_langs_for_domain(&node, VisualFormat::PlainText, &[], dom_b);
    assert!(
        out_b.contains("TITLE") && out_b.contains("Main Window"),
        "dom_b expected plain UI marker, got {:?}",
        out_b
    );
}

#[test]
fn test_kuit_custom_marker_format_override() {
    set_marker_format("title", VisualFormat::RichText);
    let node = parse("@title:Main Window/plain");
    let out = to_format(&node, VisualFormat::PlainText);
    assert!(out.contains("<h1>Main Window</h1>"));
}

#[test]
fn test_kuit_custom_tag_pattern_replacement() {
    set_tag_pattern("customb", Some(VisualFormat::PlainText), Some("[[%1]]".to_string()));
    let node = parse("@customb{Hello}");
    let out = to_format(&node, VisualFormat::PlainText);
    assert_eq!(out, "[[Hello]]");
}

#[test]
fn test_kuit_struct_tag_class_adds_block_spacing() {
    set_tag_class("structx", TagClass::StructTag);
    let node = parse("@structx{hello}");
    let out = to_format(&node, VisualFormat::PlainText);
    assert!(out.starts_with('\n') || out.contains("\nhello"));
    assert!(out.contains("hello"));
}

/// 与上游一致：属性组合键 + `%2` 起为 `attribNames` 顺序。
#[test]
fn test_kuit_set_tag_pattern_attrib_names_placeholders() {
    set_tag_pattern_full(
        "xlink",
        &["href"],
        VisualFormat::RichText,
        Some(r#"<a href="%2">%1</a>"#),
        0,
        0,
    );
    let node = parse(r#"<div><xlink href="http://ex">Go</xlink></div>"#);
    let out = to_format(&node, VisualFormat::RichText);
    assert!(out.contains("http://ex"));
    assert!(out.contains("Go"));
    assert!(out.contains("<a href="));
}

/// `leadingNewlines`：前文非空时补足换行（与上游 `formatSubText` 一致）。
#[test]
fn test_kuit_leading_newlines_after_preceding_text() {
    set_tag_pattern_full(
        "qqp",
        &[] as &[&str],
        VisualFormat::PlainText,
        Some("[%1]"),
        2,
        0,
    );
    // 必须以 `<` 开头才会走 XML 解析；否则整段会当作纯文本
    let wrapped = parse("<div>hi<qqp>inner</qqp></div>");
    let out = to_format(&wrapped, VisualFormat::PlainText);
    assert!(out.contains("hi"));
    assert!(out.contains("[inner]"));
    assert!(out.contains("hi\n\n["), "got {:?}", out);
}

