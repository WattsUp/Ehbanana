#ifndef _EHBANANA_PAGE_H_
#define _EHBANANA_PAGE_H_

#include <Ehbanana.h>

namespace Ehbanana {

/**
 * @brief Abstract class for a page
 *
 * Wraps the DLL interface in a class with templated functions
 */
class Page {
public:
  Page(const Page &) = delete;
  Page & operator=(const Page &) = delete;

  /**
   * @brief Construct a new Page object
   *
   * @param gui to communicate to
   * @param href of the page
   */
  Page(EBGUI_t gui, std::string href) : gui(gui), href(href) {}

  /**
   * @brief Destroy the Page object
   *
   */
  virtual ~Page() {}

  /**
   * @brief Handle the input from the GUI
   *
   * @param msg to handle
   * @return Result
   */
  virtual Result handleInput(const EBMessage_t & msg) = 0;

  /**
   * @brief Handle when the page first loads
   * Likely send initial values
   *
   * @return Result
   */
  virtual Result onLoad() = 0;

  /**
   * @brief Send an update to the page
   * Likely send real time values
   *
   * @return Result
   */
  virtual Result sendUpdate() = 0;

protected:
  /**
   * @brief Create a new message
   *
   * @param throwOnError, if true, will throw an exception if an error ocurred
   * @return Result
   */
  Result createNewEBMessage(bool throwOnError = true) {
    Result result = EBMessageOutCreate(gui);
    if (!result) {
      result = result + "EBMessageOutCreate";
      if (throwOnError)
        throw std::exception(result.getMessage());
      return result;
    }

    result = EBMessageOutSetHref(gui, href);
    if (!result) {
      result = result + "EBMessageOutSetHref";
      if (throwOnError)
        throw std::exception(result.getMessage());
      return result;
    }
    return ResultCode_t::SUCCESS;
  }

  /**
   * @brief Enqueue the EBMessage
   *
   * @param throwOnError, if true, will throw an exception if an error ocurred
   * @return Result
   */
  Result enqueueEBMessage(bool throwOnError = true) {
    Result result = EBMessageOutEnqueue(gui);
    if (!result) {
      result = result + "EBMessageOutEnqueue";
      if (throwOnError)
        throw std::exception(result.getMessage());
      return result;
    }
    return ResultCode_t::SUCCESS;
  }

  /**
   * @brief Set a property of the EBMessage
   *
   * @param id of the HTML element
   * @param name of the property
   * @param value of the property
   * @param throwOnError, if true, will throw an exception if an error ocurred
   * @return Result
   */
  template <typename T>
  Result messageSetProp(const std::string id, const std::string name,
      const T value, bool throwOnError = true) {
    Result result = EBMessageOutSetProp(gui, id, name, value);
    if (!result) {
      result = result + "EBMessageOutSetProp";
      if (throwOnError)
        throw std::exception(result.getMessage());
      return result;
    }
    return ResultCode_t::SUCCESS;
  }

private:
  EBGUI_t     gui = nullptr;
  std::string href;
};

} // namespace Ehbanana

#endif /* _GUI_PAGE_H_ */