// Copyright 2011 Software Freedom Conservancy
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef WEBDRIVER_IE_IECOMMANDEXECUTOR_H_
#define WEBDRIVER_IE_IECOMMANDEXECUTOR_H_

#include <Objbase.h>
#include <map>
#include <string>
#include <unordered_map>
#include "BrowserFactory.h"
#include "command.h"
#include "command_types.h"
#include "DocumentHost.h"
#include "IECommandHandler.h"
#include "Element.h"
#include "ElementFinder.h"
#include "ElementRepository.h"
#include "InputManager.h"
#include "ProxyManager.h"
#include "messages.h"
#include "response.h"

#define WAIT_TIME_IN_MILLISECONDS 200
#define FIND_ELEMENT_WAIT_TIME_IN_MILLISECONDS 250
#define ASYNC_SCRIPT_EXECUTION_TIMEOUT_IN_MILLISECONDS 2000
#define MAX_HTML_DIALOG_RETRIES 5
#define IGNORE_UNEXPECTED_ALERTS "ignore"
#define ACCEPT_UNEXPECTED_ALERTS "accept"
#define DISMISS_UNEXPECTED_ALERTS "dismiss"

namespace webdriver {

// We use a CWindowImpl (creating a hidden window) here because we
// want to synchronize access to the command handler. For that we
// use SendMessage() most of the time, and SendMessage() requires
// a window handle.
class IECommandExecutor : public CWindowImpl<IECommandExecutor> {
 public:
  DECLARE_WND_CLASS(L"WebDriverWndClass")

  BEGIN_MSG_MAP(Session)
    MESSAGE_HANDLER(WM_CREATE, OnCreate)
    MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
    MESSAGE_HANDLER(WD_SET_COMMAND, OnSetCommand)
    MESSAGE_HANDLER(WD_EXEC_COMMAND, OnExecCommand)
    MESSAGE_HANDLER(WD_GET_RESPONSE_LENGTH, OnGetResponseLength)
    MESSAGE_HANDLER(WD_GET_RESPONSE, OnGetResponse)
    MESSAGE_HANDLER(WD_WAIT, OnWait)
    MESSAGE_HANDLER(WD_BROWSER_NEW_WINDOW, OnBrowserNewWindow)
    MESSAGE_HANDLER(WD_BROWSER_QUIT, OnBrowserQuit)
    MESSAGE_HANDLER(WD_IS_SESSION_VALID, OnIsSessionValid)
    MESSAGE_HANDLER(WD_NEW_HTML_DIALOG, OnNewHtmlDialog)
    MESSAGE_HANDLER(WD_GET_QUIT_STATUS, OnGetQuitStatus)
    MESSAGE_HANDLER(WD_REFRESH_MANAGED_ELEMENTS, OnRefreshManagedElements)
    MESSAGE_HANDLER(WD_HANDLE_UNEXPECTED_ALERTS, OnHandleUnexpectedAlerts)
  END_MSG_MAP()

  LRESULT OnCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnSetCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnExecCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnGetResponseLength(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnGetResponse(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnWait(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnBrowserNewWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnBrowserQuit(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnIsSessionValid(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnNewHtmlDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnGetQuitStatus(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnRefreshManagedElements(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
  LRESULT OnHandleUnexpectedAlerts(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);

  std::string session_id(void) const { return this->session_id_; }

  static unsigned int WINAPI ThreadProc(LPVOID lpParameter);
  static unsigned int WINAPI WaitThreadProc(LPVOID lpParameter);

  std::string current_browser_id(void) const { 
    return this->current_browser_id_; 
  }
  void set_current_browser_id(const std::string& browser_id) {
    this->current_browser_id_ = browser_id;
  }

  int CreateNewBrowser(std::string* error_message);

  int GetManagedBrowser(const std::string& browser_id,
                        BrowserHandle* browser_wrapper) const;
  int GetCurrentBrowser(BrowserHandle* browser_wrapper) const;
  void GetManagedBrowserHandles(
      std::vector<std::string> *managed_browser_handles) const;

  int GetManagedElement(const std::string& element_id,
                        ElementHandle* element_wrapper) const;
  void AddManagedElement(IHTMLElement* element,
                         ElementHandle* element_wrapper);
  void RemoveManagedElement(const std::string& element_id);
  void ListManagedElements(void);

  int GetElementFindMethod(const std::string& mechanism,
                           std::wstring* translation) const;
  int LocateElement(const ElementHandle parent_wrapper,
                    const std::string& mechanism,
                    const std::string& criteria,
                    Json::Value* found_element) const;
  int LocateElements(const ElementHandle parent_wrapper,
                     const std::string& mechanism,
                     const std::string& criteria,
                     Json::Value* found_elements) const;

  int implicit_wait_timeout(void) const { 
    return this->implicit_wait_timeout_; 
  }
  void set_implicit_wait_timeout(const int timeout) { 
    this->implicit_wait_timeout_ = timeout; 
  }

  int async_script_timeout(void) const { return this->async_script_timeout_;  }
  void set_async_script_timeout(const int timeout) {
    this->async_script_timeout_ = timeout;
  }

  int page_load_timeout(void) const { return this->page_load_timeout_;  }
  void set_page_load_timeout(const int timeout) {
    this->page_load_timeout_ = timeout;
  }

  bool is_valid(void) const { return this->is_valid_; }
  void set_is_valid(const bool session_is_valid) {
    this->is_valid_ = session_is_valid; 
  }

  bool is_quitting(void) const { return this->is_quitting_; }
  void set_is_quitting(const bool session_is_quitting) {
    this->is_quitting_ = session_is_quitting; 
  }

  bool enable_element_cache_cleanup(void) const {
    return this->enable_element_cache_cleanup_;
  }
  void set_enable_element_cache_cleanup(const bool enable_element_cache_cleanup) {
    this->enable_element_cache_cleanup_ = enable_element_cache_cleanup;
  }

  bool enable_persistent_hover(void) const {
    return this->enable_persistent_hover_;
  }
  void set_enable_persistent_hover(const bool enable_persistent_hover) {
    this->enable_persistent_hover_ = enable_persistent_hover;
  }

  std::string unexpected_alert_behavior(void) const {
    return this->unexpected_alert_behavior_;
  }
  void set_unexpected_alert_behavior(const std::string& unexpected_alert_behavior) {
    this->unexpected_alert_behavior_ = unexpected_alert_behavior;
  }

  ElementFinder element_finder(void) const { return this->element_finder_; }
  InputManager* input_manager(void) const { return this->input_manager_; }
  ProxyManager* proxy_manager(void) const { return this->proxy_manager_; }
  BrowserFactory* browser_factory(void) const { return this->factory_; }

  int port(void) const { return this->port_; }

  int browser_version(void) const { return this->factory_->browser_version(); }
  size_t managed_window_count(void) const {
    return this->managed_browsers_.size();
  }

 private:
  typedef std::tr1::unordered_map<std::string, BrowserHandle> BrowserMap;
  typedef std::map<std::string, std::wstring> ElementFindMethodMap;
  typedef std::map<std::string, CommandHandlerHandle> CommandHandlerMap;

  void AddManagedBrowser(BrowserHandle browser_wrapper);

  void DispatchCommand(void);

  void PopulateCommandHandlers(void);
  void PopulateElementFinderMethods(void);

  bool IsAlertActive(BrowserHandle browser, HWND* alert_handle);
  std::string HandleUnexpectedAlert(BrowserHandle browser,
                                    HWND alert_handle,
                                    bool force_use_dismiss);

  BrowserMap managed_browsers_;
  ElementRepository managed_elements_;
  ElementFindMethodMap element_find_methods_;

  std::string current_browser_id_;

  ElementFinder element_finder_;

  int implicit_wait_timeout_;
  int async_script_timeout_;
  int page_load_timeout_;
  clock_t wait_timeout_;

  std::string session_id_;
  int port_;
  bool enable_persistent_hover_;
  bool enable_element_cache_cleanup_;
  bool ignore_zoom_setting_;
  std::string initial_browser_url_;
  std::string unexpected_alert_behavior_;

  Command current_command_;
  std::string serialized_response_;
  CommandHandlerMap command_handlers_;
  bool is_waiting_;
  bool is_valid_;
  bool is_quitting_;

  BrowserFactory* factory_;
  InputManager* input_manager_;
  ProxyManager* proxy_manager_;
};

} // namespace webdriver

#endif // WEBDRIVER_IE_IECOMMANDEXECUTOR_H_
