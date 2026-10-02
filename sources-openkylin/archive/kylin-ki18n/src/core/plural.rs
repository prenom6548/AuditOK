// SPDX-FileCopyrightText: 2021-2026 KylinSoft Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

use std::fmt;

use thiserror::Error;

/// 复数表达式 AST
#[derive(Clone, Debug)]
enum PluralExpr {
    Ternary(Box<PluralExpr>, Box<PluralExpr>, Box<PluralExpr>),
    Or(Box<PluralExpr>, Box<PluralExpr>),
    And(Box<PluralExpr>, Box<PluralExpr>),
    Eq(Box<PluralExpr>, Box<PluralExpr>, EqOp),
    Cmp(Box<PluralExpr>, Box<PluralExpr>, CmpOp),
    Add(Box<PluralExpr>, Box<PluralExpr>, AddOp),
    Mul(Box<PluralExpr>, Box<PluralExpr>, MulOp),
    Not(Box<PluralExpr>),
    Neg(Box<PluralExpr>),
    Number(i64),
    VarN,
}

#[derive(Clone, Copy, Debug)]
enum EqOp {
    Eq,
    Ne,
}

#[derive(Clone, Copy, Debug)]
enum CmpOp {
    Lt,
    Le,
    Gt,
    Ge,
}

#[derive(Clone, Copy, Debug)]
enum AddOp {
    Add,
    Sub,
}

#[derive(Clone, Copy, Debug)]
enum MulOp {
    Mul,
    Div,
    Mod,
}

/// 复数规则表示
#[derive(Clone, Debug)]
pub struct PluralRule {
    nplurals: u8,
    expr_src: String,
    expr_ast: PluralExpr,
}

#[derive(Debug, Error)]
pub enum PluralError {
    #[error("Plural-Forms header is missing or empty")]
    MissingHeader,
    #[error("Invalid nplurals value in Plural-Forms: {0}")]
    InvalidNPlurals(String),
    #[error("Invalid plural expression in Plural-Forms: {0}")]
    InvalidExpression(String),
}

impl PluralRule {
    pub fn nplurals(&self) -> u8 {
        self.nplurals
    }
}

impl fmt::Display for PluralRule {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            f,
            "PluralRule(nplurals={}, expr={})",
            self.nplurals, self.expr_src
        )
    }
}

/// 从 `Plural-Forms:` 头中解析复数规则。
///
/// 典型格式：
/// `Plural-Forms: nplurals=2; plural=(n != 1);`
pub fn parse_plural_forms(header: &str) -> Result<PluralRule, PluralError> {
    let mut nplurals: Option<u8> = None;
    let mut expr: Option<String> = None;

    for part in header.split(';') {
        let trimmed = part.trim();
        if trimmed.is_empty() {
            continue;
        }
        if let Some(rest) = trimmed.strip_prefix("nplurals=") {
            let value_str = rest.trim();
            let value = value_str
                .chars()
                .take_while(|c| c.is_ascii_digit())
                .collect::<String>();
            if value.is_empty() {
                return Err(PluralError::InvalidNPlurals(rest.to_string()));
            }
            let parsed: u8 = value
                .parse()
                .map_err(|_| PluralError::InvalidNPlurals(value.clone()))?;
            nplurals = Some(parsed);
        } else if let Some(rest) = trimmed.strip_prefix("plural=") {
            let value = rest.trim().trim_end_matches(';').trim();
            expr = Some(value.to_string());
        }
    }

    let nplurals = nplurals.ok_or_else(|| PluralError::MissingHeader)?;
    let expr_str = expr.ok_or_else(|| PluralError::MissingHeader)?;

    let ast = parse_plural_expr(&expr_str)
        .map_err(|_| PluralError::InvalidExpression(expr_str.clone()))?;

    Ok(PluralRule {
        nplurals,
        expr_src: expr_str,
        expr_ast: ast,
    })
}

/// 根据语言代码返回预定义的复数规则。
///
/// 仅针对任务中明确列出的语言保证行为与 gettext 一致。
pub fn get_plural_forms_expression(lang: &str) -> Option<PluralRule> {
    let lang = lang.to_ascii_lowercase();

    // zh/ja/ko 无复数
    if ["zh", "zh_cn", "zh_tw", "ja", "ko"]
        .iter()
        .any(|p| lang.starts_with(p))
    {
        let expr = "0".to_string();
        let ast = parse_plural_expr(&expr).ok()?;
        return Some(PluralRule {
            nplurals: 1,
            expr_src: expr,
            expr_ast: ast,
        });
    }

    // 英语族
    if ["en"].iter().any(|p| lang.starts_with(p)) {
        let expr = "(n != 1)".to_string();
        let ast = parse_plural_expr(&expr).ok()?;
        return Some(PluralRule {
            nplurals: 2,
            expr_src: expr,
            expr_ast: ast,
        });
    }

    // 俄语 / 乌克兰语
    if ["ru", "uk"].iter().any(|p| lang.starts_with(p)) {
        let expr = "(n%10==1 && n%100!=11 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2)".to_string();
        let ast = parse_plural_expr(&expr).ok()?;
        return Some(PluralRule {
            nplurals: 3,
            expr_src: expr,
            expr_ast: ast,
        });
    }

    // 波兰语
    if ["pl"].iter().any(|p| lang.starts_with(p)) {
        let expr = "(n==1 ? 0 : n%10>=2 && n%10<=4 && (n%100<10 || n%100>=20) ? 1 : 2)".to_string();
        let ast = parse_plural_expr(&expr).ok()?;
        return Some(PluralRule {
            nplurals: 3,
            expr_src: expr,
            expr_ast: ast,
        });
    }

    // 阿拉伯语
    if ["ar"].iter().any(|p| lang.starts_with(p)) {
        let expr = "(n==0 ? 0 : n==1 ? 1 : n==2 ? 2 : n%100>=3 && n%100<=10 ? 3 : n%100>=11 ? 4 : 5)".to_string();
        let ast = parse_plural_expr(&expr).ok()?;
        return Some(PluralRule {
            nplurals: 6,
            expr_src: expr,
            expr_ast: ast,
        });
    }

    None
}

/// 计算给定数字对应的复数下标。
///
/// 完整实现 gettext 复数表达式求值。
pub fn evaluate_plural(rule: &PluralRule, n: u64) -> usize {
    let value = eval_expr(&rule.expr_ast, n as i64);
    if value < 0 {
        0
    } else {
        let idx = value as usize;
        let max = rule.nplurals().saturating_sub(1) as usize;
        if idx > max {
            max
        } else {
            idx
        }
    }
}

// ---------------------------------------------------------------------------
// 递归下降解析器
// ---------------------------------------------------------------------------

fn parse_plural_expr(src: &str) -> Result<PluralExpr, ()> {
    let tokens: Vec<char> = src.chars().collect();
    let mut p = Parser { src: &tokens, pos: 0 };
    let expr = p.parse_ternary()?;
    p.skip_ws();
    if p.pos != p.src.len() {
        return Err(());
    }
    Ok(expr)
}

struct Parser<'a> {
    src: &'a [char],
    pos: usize,
}

impl<'a> Parser<'a> {
    fn peek(&self) -> Option<char> {
        self.src.get(self.pos).copied()
    }

    fn next(&mut self) -> Option<char> {
        let ch = self.peek()?;
        self.pos += 1;
        Some(ch)
    }

    fn skip_ws(&mut self) {
        while let Some(ch) = self.peek() {
            if ch.is_whitespace() {
                self.pos += 1;
            } else {
                break;
            }
        }
    }

    fn match_str(&mut self, s: &str) -> bool {
        self.skip_ws();
        if self.pos + s.len() > self.src.len() {
            return false;
        }
        if self.src[self.pos..self.pos + s.len()]
            .iter()
            .copied()
            .collect::<String>()
            == s
        {
            self.pos += s.len();
            true
        } else {
            false
        }
    }

    fn parse_ternary(&mut self) -> Result<PluralExpr, ()> {
        let mut expr = self.parse_or()?;
        self.skip_ws();
        if self.match_str("?") {
            let then_expr = self.parse_ternary()?;
            if !self.match_str(":") {
                return Err(());
            }
            let else_expr = self.parse_ternary()?;
            expr = PluralExpr::Ternary(Box::new(expr), Box::new(then_expr), Box::new(else_expr));
        }
        Ok(expr)
    }

    fn parse_or(&mut self) -> Result<PluralExpr, ()> {
        let mut expr = self.parse_and()?;
        loop {
            self.skip_ws();
            if self.match_str("||") {
                let rhs = self.parse_and()?;
                expr = PluralExpr::Or(Box::new(expr), Box::new(rhs));
            } else {
                break;
            }
        }
        Ok(expr)
    }

    fn parse_and(&mut self) -> Result<PluralExpr, ()> {
        let mut expr = self.parse_eq()?;
        loop {
            self.skip_ws();
            if self.match_str("&&") {
                let rhs = self.parse_eq()?;
                expr = PluralExpr::And(Box::new(expr), Box::new(rhs));
            } else {
                break;
            }
        }
        Ok(expr)
    }

    fn parse_eq(&mut self) -> Result<PluralExpr, ()> {
        let mut expr = self.parse_cmp()?;
        loop {
            self.skip_ws();
            if self.match_str("==") {
                let rhs = self.parse_cmp()?;
                expr = PluralExpr::Eq(Box::new(expr), Box::new(rhs), EqOp::Eq);
            } else if self.match_str("!=") {
                let rhs = self.parse_cmp()?;
                expr = PluralExpr::Eq(Box::new(expr), Box::new(rhs), EqOp::Ne);
            } else {
                break;
            }
        }
        Ok(expr)
    }

    fn parse_cmp(&mut self) -> Result<PluralExpr, ()> {
        let mut expr = self.parse_add()?;
        loop {
            self.skip_ws();
            if self.match_str("<=") {
                let rhs = self.parse_add()?;
                expr = PluralExpr::Cmp(Box::new(expr), Box::new(rhs), CmpOp::Le);
            } else if self.match_str(">=") {
                let rhs = self.parse_add()?;
                expr = PluralExpr::Cmp(Box::new(expr), Box::new(rhs), CmpOp::Ge);
            } else if self.match_str("<") {
                let rhs = self.parse_add()?;
                expr = PluralExpr::Cmp(Box::new(expr), Box::new(rhs), CmpOp::Lt);
            } else if self.match_str(">") {
                let rhs = self.parse_add()?;
                expr = PluralExpr::Cmp(Box::new(expr), Box::new(rhs), CmpOp::Gt);
            } else {
                break;
            }
        }
        Ok(expr)
    }

    fn parse_add(&mut self) -> Result<PluralExpr, ()> {
        let mut expr = self.parse_mul()?;
        loop {
            self.skip_ws();
            if self.match_str("+") {
                let rhs = self.parse_mul()?;
                expr = PluralExpr::Add(Box::new(expr), Box::new(rhs), AddOp::Add);
            } else if self.match_str("-") {
                let rhs = self.parse_mul()?;
                expr = PluralExpr::Add(Box::new(expr), Box::new(rhs), AddOp::Sub);
            } else {
                break;
            }
        }
        Ok(expr)
    }

    fn parse_mul(&mut self) -> Result<PluralExpr, ()> {
        let mut expr = self.parse_unary()?;
        loop {
            self.skip_ws();
            if self.match_str("*") {
                let rhs = self.parse_unary()?;
                expr = PluralExpr::Mul(Box::new(expr), Box::new(rhs), MulOp::Mul);
            } else if self.match_str("/") {
                let rhs = self.parse_unary()?;
                expr = PluralExpr::Mul(Box::new(expr), Box::new(rhs), MulOp::Div);
            } else if self.match_str("%") {
                let rhs = self.parse_unary()?;
                expr = PluralExpr::Mul(Box::new(expr), Box::new(rhs), MulOp::Mod);
            } else {
                break;
            }
        }
        Ok(expr)
    }

    fn parse_unary(&mut self) -> Result<PluralExpr, ()> {
        self.skip_ws();
        if self.match_str("!") {
            let inner = self.parse_unary()?;
            Ok(PluralExpr::Not(Box::new(inner)))
        } else if self.match_str("-") {
            let inner = self.parse_unary()?;
            Ok(PluralExpr::Neg(Box::new(inner)))
        } else {
            self.parse_primary()
        }
    }

    fn parse_primary(&mut self) -> Result<PluralExpr, ()> {
        self.skip_ws();
        match self.peek() {
            Some('n') => {
                self.next();
                Ok(PluralExpr::VarN)
            }
            Some('0'..='9') => {
                let mut value: i64 = 0;
                while let Some(ch) = self.peek() {
                    if ch.is_ascii_digit() {
                        self.next();
                        value = value * 10 + (ch as i64 - '0' as i64);
                    } else {
                        break;
                    }
                }
                Ok(PluralExpr::Number(value))
            }
            Some('(') => {
                self.next();
                let expr = self.parse_ternary()?;
                self.skip_ws();
                if self.peek() == Some(')') {
                    self.next();
                    Ok(expr)
                } else {
                    Err(())
                }
            }
            _ => Err(()),
        }
    }
}

fn eval_expr(expr: &PluralExpr, n: i64) -> i64 {
    match expr {
        PluralExpr::Ternary(cond, then_e, else_e) => {
            let c = eval_expr(cond, n);
            if c != 0 {
                eval_expr(then_e, n)
            } else {
                eval_expr(else_e, n)
            }
        }
        PluralExpr::Or(a, b) => {
            let av = eval_expr(a, n);
            if av != 0 {
                1
            } else {
                let bv = eval_expr(b, n);
                if bv != 0 {
                    1
                } else {
                    0
                }
            }
        }
        PluralExpr::And(a, b) => {
            let av = eval_expr(a, n);
            if av == 0 {
                0
            } else {
                let bv = eval_expr(b, n);
                if bv != 0 {
                    1
                } else {
                    0
                }
            }
        }
        PluralExpr::Eq(a, b, op) => {
            let av = eval_expr(a, n);
            let bv = eval_expr(b, n);
            match op {
                EqOp::Eq => {
                    if av == bv {
                        1
                    } else {
                        0
                    }
                }
                EqOp::Ne => {
                    if av != bv {
                        1
                    } else {
                        0
                    }
                }
            }
        }
        PluralExpr::Cmp(a, b, op) => {
            let av = eval_expr(a, n);
            let bv = eval_expr(b, n);
            let res = match op {
                CmpOp::Lt => av < bv,
                CmpOp::Le => av <= bv,
                CmpOp::Gt => av > bv,
                CmpOp::Ge => av >= bv,
            };
            if res {
                1
            } else {
                0
            }
        }
        PluralExpr::Add(a, b, op) => {
            let av = eval_expr(a, n);
            let bv = eval_expr(b, n);
            match op {
                AddOp::Add => av + bv,
                AddOp::Sub => av - bv,
            }
        }
        PluralExpr::Mul(a, b, op) => {
            let av = eval_expr(a, n);
            let bv = eval_expr(b, n);
            match op {
                MulOp::Mul => av * bv,
                MulOp::Div => {
                    if bv == 0 {
                        0
                    } else {
                        av / bv
                    }
                }
                MulOp::Mod => {
                    if bv == 0 {
                        0
                    } else {
                        av % bv
                    }
                }
            }
        }
        PluralExpr::Not(e) => {
            let v = eval_expr(e, n);
            if v == 0 {
                1
            } else {
                0
            }
        }
        PluralExpr::Neg(e) => -eval_expr(e, n),
        PluralExpr::Number(v) => *v,
        PluralExpr::VarN => n,
    }
}

