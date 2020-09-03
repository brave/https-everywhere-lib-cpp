use std::collections::HashMap;
use std::error::Error;
use std::ffi::{CStr, CString};
use std::os::raw::c_char;
use std::sync::{Arc, Mutex};

use https_everywhere_lib_core::{
    rewriter::RewriteAction, rulesets::RuleSets, settings::Settings, Rewriter,
};

/// Main struct accessible as a pointer across the FFI.
///
/// `rewriter` is `None` until rules have been added for initialization.
#[derive(Default)]
pub struct HttpseClient {
    rewriter: Option<Rewriter>,
}

/// Creates a new HttpseClient for use across the FFI.
#[no_mangle]
pub extern "C" fn new_client() -> *mut HttpseClient {
    Box::into_raw(Box::new(HttpseClient::default()))
}

/// Initializes the `HttpseClient` with a set of rules, in JSON format.
///
/// The rules are in the same format as the `rulesets` key of the EFF's official lists.
///
/// Returns whether or not the operation was successful.
///
/// # Safety
/// This function will cause undefined behavior if `client` or `rules` do not point to properly
/// initialized memory.
#[no_mangle]
pub unsafe extern "C" fn initialize_client(
    client: *mut HttpseClient,
    rules: *const c_char,
) -> bool {
    let rules = CStr::from_ptr(rules)
        .to_str()
        .expect("Convert rules CStr to str");
    match serde_json::from_str(rules) {
        Ok(parsed_rulesets) => {
            let mut rulesets = RuleSets::new();
            rulesets.add_all_from_serde_value(parsed_rulesets, false, &HashMap::new(), &None);
            let rulesets = Arc::new(Mutex::new(rulesets));
            let settings = Arc::new(Mutex::new(Settings::new(Arc::new(Mutex::new(
                FixedSettings,
            )))));
            (*client).rewriter = Some(Rewriter::new(rulesets, settings));
        }
        Err(e) => {
            eprintln!("Error parsing HTTPS Everywhere rulesets: {}", e);
            return false;
        }
    }
    true
}

/// Use the `HttpseClient` to rewrite the given URL according to any applicable rules.
///
/// # Safety
/// This function will cause undefined behavior if `client` or `url` do not point to properly
/// initialized memory.
#[no_mangle]
pub unsafe extern "C" fn rewriter_rewrite_url(
    client: *mut HttpseClient,
    url: *const c_char,
) -> RewriteResult {
    let client = Box::leak(Box::from_raw(client));
    if let Some(rewriter) = client.rewriter.as_mut() {
        let url = CStr::from_ptr(url)
            .to_str()
            .expect("Convert url CStr to str");
        rewriter.rewrite_url(url).into()
    } else {
        eprintln!("Attempted to access uninitialized HTTPSE client");
        RewriteResult {
            action: RewriteActionEnum::NoOp,
            new_url: CString::default().into_raw(),
        }
    }
}

/// A C-compatible return type representing the result of a rewrite operation.
#[repr(C)]
pub struct RewriteResult {
    action: RewriteActionEnum,
    new_url: *const c_char,
}

/// A C-compatible enumerated type representing possible actions taken as the result of a rewrite
/// operation.
#[repr(C)]
pub enum RewriteActionEnum {
    NoOp = 0,
    RewriteUrl = 1,
}

/// A `RewriteResult` can be easily created from the `<Result<RewriteAction, Box<dyn Error>>>`
/// returned by `Rewriter::rewrite_url`.
impl From<Result<RewriteAction, Box<dyn Error>>> for RewriteResult {
    fn from(v: Result<RewriteAction, Box<dyn Error>>) -> Self {
        match v {
            Ok(RewriteAction::CancelRequest) => RewriteResult {
                action: RewriteActionEnum::NoOp,
                new_url: CString::default().into_raw(),
            },
            Ok(RewriteAction::NoOp) => RewriteResult {
                action: RewriteActionEnum::NoOp,
                new_url: CString::default().into_raw(),
            },
            Ok(RewriteAction::RewriteUrl(new_url)) => RewriteResult {
                action: RewriteActionEnum::RewriteUrl,
                new_url: CString::new(new_url)
                    .expect("Create new CString")
                    .into_raw(),
            },
            Ok(RewriteAction::RedirectLoopWarning) => RewriteResult {
                action: RewriteActionEnum::NoOp,
                new_url: CString::default().into_raw(),
            },
            Err(e) => {
                eprintln!("Failed to rewrite url: {}", e);
                RewriteResult {
                    action: RewriteActionEnum::NoOp,
                    new_url: CString::default().into_raw(),
                }
            }
        }
    }
}

/// Global settings used by the core library's `Rewriter`.
///
/// Currently, configuring these settings is unsupported.
struct FixedSettings;
impl https_everywhere_lib_core::Storage for FixedSettings {
    fn get_int(&self, _key: String) -> Option<usize> {
        None
    }
    fn set_int(&mut self, _key: String, _value: usize) {}
    fn get_string(&self, _key: String) -> Option<String> {
        None
    }
    fn set_string(&mut self, _key: String, _value: String) {}
    fn get_bool(&self, key: String) -> Option<bool> {
        match key.as_ref() {
            "global_enabled" => Some(true),
            "http_nowhere_on" => Some(false),
            _ => None,
        }
    }
    fn set_bool(&mut self, _key: String, _value: bool) {}
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_trivial_data() {
        let rules = r#"[{"name":"01.org","target":["01.org","www.01.org","download.01.org","lists.01.org","ml01.01.org"],"securecookie":[{"host":".+","name":".+"}],"rule":[{"from":"^http:","to":"https:"}]},{"name":"0bin.net","target":["0bin.net","www.0bin.net"],"securecookie":[{"host":"^0bin\\.net","name":".+"}],"rule":[{"from":"^http://www\\.0bin\\.net/","to":"https://0bin.net/"},{"from":"^http:","to":"https:"}]}]"#;
        let mut client = HttpseClient::default();

        let mut rulesets = RuleSets::new();
        rulesets.add_all_from_json_string(&rules, false, &HashMap::new(), &None);
        let rulesets = Arc::new(Mutex::new(rulesets));
        let settings = Arc::new(Mutex::new(Settings::new(Arc::new(Mutex::new(
            FixedSettings,
        )))));
        client.rewriter = Some(Rewriter::new(rulesets, settings));

        let action = client
            .rewriter
            .expect("attempt to access uninitialized client")
            .rewrite_url("http://01.org/")
            .expect("Successful rewrite");

        match action {
            RewriteAction::NoOp => panic!("Should not be NoOp"),
            RewriteAction::CancelRequest => panic!("Should not be CancelRequest"),
            RewriteAction::RewriteUrl(s) => assert_eq!(s, "https://01.org/"),
            RewriteAction::RedirectLoopWarning => panic!("Should not be RedirectLoopWarning"),
        }
    }

    #[test]
    fn test_real_data() {
        let rules = std::fs::read_to_string("./data/httpse.json").expect("Read rules from file");
        let mut client = HttpseClient::default();

        let mut rulesets = RuleSets::new();
        rulesets.add_all_from_json_string(&rules, false, &HashMap::new(), &None);
        let rulesets = Arc::new(Mutex::new(rulesets));
        let settings = Arc::new(Mutex::new(Settings::new(Arc::new(Mutex::new(
            FixedSettings,
        )))));
        client.rewriter = Some(Rewriter::new(rulesets, settings));

        let action = client
            .rewriter
            .expect("attempt to access uninitialized client")
            .rewrite_url("http://01.org/")
            .expect("Successful rewrite");

        match action {
            RewriteAction::NoOp => panic!("Should not be NoOp"),
            RewriteAction::CancelRequest => panic!("Should not be CancelRequest"),
            RewriteAction::RewriteUrl(s) => assert_eq!(s, "https://01.org/"),
            RewriteAction::RedirectLoopWarning => panic!("Should not be RedirectLoopWarning"),
        }
    }
}
