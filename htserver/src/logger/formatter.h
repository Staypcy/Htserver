#ifndef HTSERVER_SRC_LOGGER_FORMATTER_H
#define HTSERVER_SRC_LOGGER_FORMATTER_H

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "logger/buffer.h"
#include "logger/buffer_config.h"
#include "logger/log_record.h"

namespace htserver::detail {

using FormattedRecordBuffer = InlineBuffer<FORMATTED_RECORD_INLINE_CAPACITY>;

class Formatter final
{
public:
    class FormatItem
    {
    public:
        virtual ~FormatItem() = default;
        virtual void format(const LogRecordView& record, FormattedRecordBuffer& output) const = 0;
    };

    explicit Formatter(std::string_view pattern);

    //该函数最好不要加上noexcept修饰，因为在极端情况下可能Inlinebuffer的vector分配不到足够的内存空间
    void format(const LogRecordView& record, FormattedRecordBuffer& output) const;

    [[nodiscard]] const std::string& getPattern() const noexcept { return m_pattern; }

private:
    int parse();
    void addLiteral(std::string& literal);

private:
    std::string m_pattern;
    std::vector<std::unique_ptr<FormatItem>> m_items;
};

} // namespace htserver::detail

#endif // HTSERVER_SRC_LOGGER_FORMATTER_H
