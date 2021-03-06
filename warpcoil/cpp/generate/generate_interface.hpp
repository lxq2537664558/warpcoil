#pragma once

#include <warpcoil/cpp/generate/shared_code_generator.hpp>
#include <warpcoil/block.hpp>

namespace warpcoil
{
    namespace cpp
    {
        template <class CharSink>
        void move_arguments_out_of_tuple(CharSink &&code, Si::memory_range tuple_name,
                                         std::size_t count)
        {
            for (size_t i = 0; i < count; ++i)
            {
                using Si::append;
                append(code, "std::get<");
                append(code, boost::lexical_cast<std::string>(i));
                append(code, ">(std::move(");
                append(code, tuple_name);
                append(code, ")), ");
            }
        }

        template <class CharSink1, class CharSink2>
        void generate_interface(CharSink1 &&code, shared_code_generator<CharSink2> &shared,
                                indentation_level indentation, Si::memory_range name,
                                types::interface_definition const &definition)
        {
            using Si::append;
            append(code, "struct async_");
            append(code, name);
            append(code, "\n");
            block(code, indentation,
                  [&](indentation_level const in_class)
                  {
                      in_class.render(code);
                      append(code, "virtual ~async_");
                      append(code, name);
                      append(code, "() {}\n\n");
                      for (auto const &entry : definition.methods)
                      {
                          in_class.render(code);
                          append(code, "virtual void ");
                          append(code, entry.first);
                          append(code, "(");
                          generate_parameters(code, shared, entry.second.parameters);
                          append(code, "std::function<void(Si::error_or<");
                          generate_type(code, shared, entry.second.result);
                          append(code, ">)> on_result) = 0;\n");
                      }
                  },
                  ";\n\n");
        }
    }
}
