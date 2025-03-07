#pragma once

#include <AK/Function.h>
#include <AK/HashMap.h>
#include <AK/OwnPtr.h>
#include <AK/WeakPtr.h>
#include <LibCore/CObject.h>
#include <LibCore/CoreIPCServer.h>
#include <LibDraw/GraphicsBitmap.h>
#include <WindowServer/WSAPITypes.h>
#include <WindowServer/WSEvent.h>

class WSWindow;
class WSMenu;
class WSMenuBar;

class WSClientConnection final : public IPC::Server::Connection<WSAPI_ServerMessage, WSAPI_ClientMessage> {
    C_OBJECT(WSClientConnection)
public:
    explicit WSClientConnection(CLocalSocket&, int client_id);
    ~WSClientConnection() override;
    void send_greeting() override;
    bool handle_message(const WSAPI_ClientMessage&, const ByteBuffer&& = {}) override;

    static WSClientConnection* from_client_id(int client_id);
    static void for_each_client(Function<void(WSClientConnection&)>);

    WSMenuBar* app_menubar() { return m_app_menubar.ptr(); }

    bool is_showing_modal_window() const;

    template<typename Matching, typename Callback>
    void for_each_window_matching(Matching, Callback);
    template<typename Callback>
    void for_each_window(Callback);

    void notify_about_new_screen_rect(const Rect&);
    void post_paint_message(WSWindow&);

    WSMenu* find_menu_by_id(int menu_id)
    {
        // FIXME: Remove this const_cast when Optional knows how to vend a non-const fallback value somehow.
        return const_cast<WSMenu*>(m_menus.get(menu_id).value_or(nullptr));
    }

private:
    virtual void event(CEvent&) override;

    void on_request(const WSAPIClientRequest&);
    void handle_request(const WSAPICreateMenubarRequest&);
    void handle_request(const WSAPIDestroyMenubarRequest&);
    void handle_request(const WSAPICreateMenuRequest&);
    void handle_request(const WSAPIDestroyMenuRequest&);
    void handle_request(const WSAPISetApplicationMenubarRequest&);
    void handle_request(const WSAPIAddMenuToMenubarRequest&);
    void handle_request(const WSAPIAddMenuItemRequest&);
    void handle_request(const WSAPIUpdateMenuItemRequest&);
    void handle_request(const WSAPIAddMenuSeparatorRequest&);
    void handle_request(const WSAPISetWindowTitleRequest&);
    void handle_request(const WSAPIGetWindowTitleRequest&);
    void handle_request(const WSAPISetWindowRectRequest&);
    void handle_request(const WSAPIGetWindowRectRequest&);
    void handle_request(const WSAPISetWindowIconBitmapRequest&);
    void handle_request(const WSAPISetClipboardContentsRequest&);
    void handle_request(const WSAPIGetClipboardContentsRequest&);
    void handle_request(const WSAPICreateWindowRequest&);
    void handle_request(const WSAPIDestroyWindowRequest&);
    void handle_request(const WSAPIInvalidateRectRequest&);
    void handle_request(const WSAPIDidFinishPaintingNotification&);
    void handle_request(const WSAPIGetWindowBackingStoreRequest&);
    void handle_request(const WSAPISetWindowBackingStoreRequest&);
    void handle_request(const WSAPISetGlobalCursorTrackingRequest&);
    void handle_request(const WSAPISetWindowOpacityRequest&);
    void handle_request(const WSAPISetWallpaperRequest&);
    void handle_request(const WSAPIGetWallpaperRequest&);
    void handle_request(const WSAPISetWindowOverrideCursorRequest&);
    void handle_request(const WSWMAPISetActiveWindowRequest&);
    void handle_request(const WSWMAPISetWindowMinimizedRequest&);
    void handle_request(const WSWMAPIStartWindowResizeRequest&);
    void handle_request(const WSWMAPIPopupWindowMenuRequest&);
    void handle_request(const WSAPIPopupMenuRequest&);
    void handle_request(const WSAPIDismissMenuRequest&);
    void handle_request(const WSAPISetWindowHasAlphaChannelRequest&);
    void handle_request(const WSAPIMoveWindowToFrontRequest&);

    void post_error(const String&);

    HashMap<int, NonnullOwnPtr<WSWindow>> m_windows;
    HashMap<int, NonnullOwnPtr<WSMenuBar>> m_menubars;
    HashMap<int, NonnullOwnPtr<WSMenu>> m_menus;
    WeakPtr<WSMenuBar> m_app_menubar;

    int m_next_menubar_id { 10000 };
    int m_next_menu_id { 20000 };
    int m_next_window_id { 1982 };

    RefPtr<SharedBuffer> m_last_sent_clipboard_content;
};

template<typename Matching, typename Callback>
void WSClientConnection::for_each_window_matching(Matching matching, Callback callback)
{
    for (auto& it : m_windows) {
        if (matching(*it.value)) {
            if (callback(*it.value) == IterationDecision::Break)
                return;
        }
    }
}

template<typename Callback>
void WSClientConnection::for_each_window(Callback callback)
{
    for (auto& it : m_windows) {
        if (callback(*it.value) == IterationDecision::Break)
            return;
    }
}
