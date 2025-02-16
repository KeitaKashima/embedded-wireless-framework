/************************************************************************//**
 * @file
 * @version Preview
 * @copyright Copyright (c) Microsoft Corporation. All rights reserved.
 * SPDX-License-Identifier: MIT
 * brief The Embedded Wireless Framework Renesas common adapter control driver
 ****************************************************************************/

#include "ewf_adapter_renesas_common.h"
#include "ewf_platform.h"
#include "ewf_lib.h"

ewf_adapter_api_control ewf_adapter_renesas_common_api_control =
{
    ewf_adapter_renesas_common_start,
    ewf_adapter_renesas_common_stop,
};

char ewf_adapter_renesas_common_command_response_end_tokenizer_pattern5_str[] = "\r\n+CME ERROR: ???\r\n";
char ewf_adapter_renesas_common_command_response_end_tokenizer_pattern4_str[] = "\r\n+CME ERROR: ??\r\n";
char ewf_adapter_renesas_common_command_response_end_tokenizer_pattern3_str[] = "\r\n+CME ERROR: ?\r\n";
char ewf_adapter_renesas_common_command_response_end_tokenizer_pattern2_str[] = "\r\nERROR\r\n";
char ewf_adapter_renesas_common_command_response_end_tokenizer_pattern1_str[] = "\r\nOK\r\n";

static ewf_interface_tokenizer_pattern ewf_adapter_renesas_common_command_response_end_tokenizer_pattern5 =
{
    NULL,
    ewf_adapter_renesas_common_command_response_end_tokenizer_pattern5_str,
    sizeof(ewf_adapter_renesas_common_command_response_end_tokenizer_pattern5_str) - 1,
    true,
    NULL,
    NULL,
};

static ewf_interface_tokenizer_pattern ewf_adapter_renesas_common_command_response_end_tokenizer_pattern4 =
{
    &ewf_adapter_renesas_common_command_response_end_tokenizer_pattern5,
    ewf_adapter_renesas_common_command_response_end_tokenizer_pattern4_str,
    sizeof(ewf_adapter_renesas_common_command_response_end_tokenizer_pattern4_str) - 1,
    true,
    NULL,
    NULL,
};

static ewf_interface_tokenizer_pattern ewf_adapter_renesas_common_command_response_end_tokenizer_pattern3 =
{
    &ewf_adapter_renesas_common_command_response_end_tokenizer_pattern4,
    ewf_adapter_renesas_common_command_response_end_tokenizer_pattern3_str,
    sizeof(ewf_adapter_renesas_common_command_response_end_tokenizer_pattern3_str) - 1,
    true,
    NULL,
    NULL,
};

static ewf_interface_tokenizer_pattern ewf_adapter_renesas_common_command_response_end_tokenizer_pattern2 =
{
    &ewf_adapter_renesas_common_command_response_end_tokenizer_pattern3,
    ewf_adapter_renesas_common_command_response_end_tokenizer_pattern2_str,
    sizeof(ewf_adapter_renesas_common_command_response_end_tokenizer_pattern2_str) - 1,
    false,
    NULL,
    NULL,
};

static ewf_interface_tokenizer_pattern ewf_adapter_renesas_common_command_response_end_tokenizer_pattern1 =
{
    &ewf_adapter_renesas_common_command_response_end_tokenizer_pattern2,
    ewf_adapter_renesas_common_command_response_end_tokenizer_pattern1_str,
    sizeof(ewf_adapter_renesas_common_command_response_end_tokenizer_pattern1_str) - 1,
    false,
    NULL,
    NULL,
};

static ewf_interface_tokenizer_pattern* ewf_adapter_renesas_common_command_response_end_tokenizer_pattern_ptr = &ewf_adapter_renesas_common_command_response_end_tokenizer_pattern1;

char ewf_adapter_renesas_common_urc_tokenizer_pattern1_str[] = "\r\n";

static ewf_interface_tokenizer_pattern ewf_adapter_renesas_common_urc_tokenizer_pattern1 =
{
    NULL,
    ewf_adapter_renesas_common_urc_tokenizer_pattern1_str,
    sizeof(ewf_adapter_renesas_common_urc_tokenizer_pattern1_str) - 1,
    false,
    NULL,
    NULL,
};

static ewf_interface_tokenizer_pattern* ewf_adapter_renesas_common_urc_tokenizer_pattern_ptr = &ewf_adapter_renesas_common_urc_tokenizer_pattern1;

/*
 * Note:
 * This custom function matching pattern is not reentrant, it can be used by only one interface at a time.
 * If you have more than one adapter at the same time, you will need to provide separate state for each one.
 * The code is prepared for that. Use an instance specific state structure instead of the global static here.
 */

struct _ewf_adapter_renesas_common_message_tokenizer_pattern7_match_function_state
{
    ewf_interface* interface_ptr;
    bool prefix_matches;
};

static struct _ewf_adapter_renesas_common_message_tokenizer_pattern7_match_function_state ewf_adapter_renesas_common_message_tokenizer_pattern7_match_function_state = { 0 };

static bool _ewf_adapter_renesas_common_message_tokenizer_pattern7_match_function(const uint8_t* buffer_ptr, uint32_t buffer_length, const ewf_interface_tokenizer_pattern* pattern_ptr, bool* stop_ptr)
{
    if (!buffer_ptr) return false;
    if (!buffer_length) return false;
    if (!pattern_ptr) return false;
    if (!stop_ptr) return false;

    struct _ewf_adapter_renesas_common_message_tokenizer_pattern7_match_function_state* state_ptr =
        (struct _ewf_adapter_renesas_common_message_tokenizer_pattern7_match_function_state*)pattern_ptr->data_ptr;

    /* Initialize the state on a new buffer */
    if (buffer_length == 1)
    {
        state_ptr->prefix_matches = false;
        return false;
    }

    /* Add a NULL terminator - explicit const override */
    ((char*)buffer_ptr)[buffer_length] = 0;

    const char prefix_str[] = "\r\n+SQNSMQTTDISCONNECT:";
    const uint32_t prefix_length = sizeof(prefix_str) - 1;

    if (buffer_length < prefix_length)
    {
        return false;
    }

    if (buffer_length == prefix_length)
    {
        if (ewfl_buffer_equals_buffer(buffer_ptr, prefix_str, prefix_length))
        {
            state_ptr->prefix_matches = true;
            return false;
        }
    }

    /* At this point the buffer it is longer than the prefix */

    /* We did not match the prefix in previous runs, just ignore the rest of the incoming characters */
    if (!state_ptr->prefix_matches)
    {
        return false;
    }
    else
    {
        /* This is for us, stop parsing other tokens further down the list */
        *stop_ptr = true;
    }

    /* At this point we have a matching prefix */

    /* Is the message complete? */
    if (buffer_ptr[buffer_length - 2] == '\r' && buffer_ptr[buffer_length - 1] == '\n')
    {
        /* Set the interface to URC mode */
        if (state_ptr->interface_ptr) state_ptr->interface_ptr->command_mode = false;

        /* Signal the match */
        return true;
    }
    else
    {
        /* Not yet matched */
        return false;
    }
}

static ewf_interface_tokenizer_pattern ewf_adapter_renesas_common_message_tokenizer_pattern7 =
{
    NULL,
    NULL,
    0,
    false,
    _ewf_adapter_renesas_common_message_tokenizer_pattern7_match_function,
    &ewf_adapter_renesas_common_message_tokenizer_pattern7_match_function_state
};

struct _ewf_adapter_renesas_common_message_tokenizer_pattern6_match_function_state
{
    ewf_interface* interface_ptr;
    bool prefix_matches;
};

static struct _ewf_adapter_renesas_common_message_tokenizer_pattern6_match_function_state ewf_adapter_renesas_common_message_tokenizer_pattern6_match_function_state = { 0 };

static bool _ewf_adapter_renesas_common_message_tokenizer_pattern6_match_function(const uint8_t* buffer_ptr, uint32_t buffer_length, const ewf_interface_tokenizer_pattern* pattern_ptr, bool* stop_ptr)
{
    if (!buffer_ptr) return false;
    if (!buffer_length) return false;
    if (!pattern_ptr) return false;
    if (!stop_ptr) return false;

    struct _ewf_adapter_renesas_common_message_tokenizer_pattern6_match_function_state* state_ptr =
        (struct _ewf_adapter_renesas_common_message_tokenizer_pattern6_match_function_state*)pattern_ptr->data_ptr;

    /* Initialize the state on a new buffer */
    if (buffer_length == 1)
    {
        state_ptr->prefix_matches = false;
        return false;
    }

    /* Add a NULL terminator - explicit const override */
    ((char*)buffer_ptr)[buffer_length] = 0;

    const char prefix_str[] = "\r\n+SQNSMQTTONCONNECT:";
    const uint32_t prefix_length = sizeof(prefix_str) - 1;

    if (buffer_length < prefix_length)
    {
        return false;
    }

    if (buffer_length == prefix_length)
    {
        if (ewfl_buffer_equals_buffer(buffer_ptr, prefix_str, prefix_length))
        {
            state_ptr->prefix_matches = true;
            return false;
        }
    }

    /* At this point the buffer it is longer than the prefix */

    /* We did not match the prefix in previous runs, just ignore the rest of the incoming characters */
    if (!state_ptr->prefix_matches)
    {
        return false;
    }
    else
    {
        /* This is for us, stop parsing other tokens further down the list */
        *stop_ptr = true;
    }

    /* At this point we have a matching prefix */

    /* Is the message complete? */
    if (buffer_ptr[buffer_length - 2] == '\r' && buffer_ptr[buffer_length - 1] == '\n')
    {
        /* Set the interface to URC mode */
        if (state_ptr->interface_ptr) state_ptr->interface_ptr->command_mode = false;

        /* Signal the match */
        return true;
    }
    else
    {
        /* Not yet matched */
        return false;
    }
}

static ewf_interface_tokenizer_pattern ewf_adapter_renesas_common_message_tokenizer_pattern6 =
{
    &ewf_adapter_renesas_common_message_tokenizer_pattern7,
    NULL,
    0,
    false,
    _ewf_adapter_renesas_common_message_tokenizer_pattern6_match_function,
    &ewf_adapter_renesas_common_message_tokenizer_pattern6_match_function_state
};

struct _ewf_adapter_renesas_common_message_tokenizer_pattern5_match_function_state
{
    ewf_interface* interface_ptr;
    bool prefix_matches;
};

static struct _ewf_adapter_renesas_common_message_tokenizer_pattern5_match_function_state ewf_adapter_renesas_common_message_tokenizer_pattern5_match_function_state = { 0 };

static bool _ewf_adapter_renesas_common_message_tokenizer_pattern5_match_function(const uint8_t* buffer_ptr, uint32_t buffer_length, const ewf_interface_tokenizer_pattern* pattern_ptr, bool* stop_ptr)
{
    if (!buffer_ptr) return false;
    if (!buffer_length) return false;
    if (!pattern_ptr) return false;
    if (!stop_ptr) return false;

    struct _ewf_adapter_renesas_common_message_tokenizer_pattern5_match_function_state* state_ptr =
        (struct _ewf_adapter_renesas_common_message_tokenizer_pattern5_match_function_state*)pattern_ptr->data_ptr;

    /* Initialize the state on a new buffer */
    if (buffer_length == 1)
    {
        state_ptr->prefix_matches = false;
        return false;
    }

    /* Add a NULL terminator - explicit const override */
    ((char*)buffer_ptr)[buffer_length] = 0;

    const char prefix_str[] = "\r\n+SQNSMQTTONSUBSCRIBE:";
    const uint32_t prefix_length = sizeof(prefix_str) - 1;

    if (buffer_length < prefix_length)
    {
        return false;
    }

    if (buffer_length == prefix_length)
    {
        if (ewfl_buffer_equals_buffer(buffer_ptr, prefix_str, prefix_length))
        {
            state_ptr->prefix_matches = true;
            return false;
        }
    }

    /* At this point the buffer it is longer than the prefix */

    /* We did not match the prefix in previous runs, just ignore the rest of the incoming characters */
    if (!state_ptr->prefix_matches)
    {
        return false;
    }
    else
    {
        /* This is for us, stop parsing other tokens further down the list */
        *stop_ptr = true;
    }

    /* At this point we have a matching prefix */

    /* Is the message complete? */
    if (buffer_ptr[buffer_length - 2] == '\r' && buffer_ptr[buffer_length - 1] == '\n')
    {
        /* Set the interface to URC mode */
        if (state_ptr->interface_ptr) state_ptr->interface_ptr->command_mode = false;

        /* Signal the match */
        return true;
    }
    else
    {
        /* Not yet matched */
        return false;
    }
}

static ewf_interface_tokenizer_pattern ewf_adapter_renesas_common_message_tokenizer_pattern5 =
{
    &ewf_adapter_renesas_common_message_tokenizer_pattern6,
    NULL,
    0,
    false,
    _ewf_adapter_renesas_common_message_tokenizer_pattern5_match_function,
    &ewf_adapter_renesas_common_message_tokenizer_pattern5_match_function_state
};

struct _ewf_adapter_renesas_common_message_tokenizer_pattern4_match_function_state
{
    ewf_interface* interface_ptr;
    bool prefix_matches;
};

static struct _ewf_adapter_renesas_common_message_tokenizer_pattern4_match_function_state ewf_adapter_renesas_common_message_tokenizer_pattern4_match_function_state = { 0 };

static bool _ewf_adapter_renesas_common_message_tokenizer_pattern4_match_function(const uint8_t* buffer_ptr, uint32_t buffer_length, const ewf_interface_tokenizer_pattern* pattern_ptr, bool* stop_ptr)
{
    if (!buffer_ptr) return false;
    if (!buffer_length) return false;
    if (!pattern_ptr) return false;
    if (!stop_ptr) return false;

    struct _ewf_adapter_renesas_common_message_tokenizer_pattern4_match_function_state* state_ptr =
        (struct _ewf_adapter_renesas_common_message_tokenizer_pattern4_match_function_state*)pattern_ptr->data_ptr;

    /* Initialize the state on a new buffer */
    if (buffer_length == 1)
    {
        state_ptr->prefix_matches = false;
        return false;
    }

    /* Add a NULL terminator - explicit const override */
    ((char*)buffer_ptr)[buffer_length] = 0;

    const char prefix_str[] = "\r\n+SQNSMQTTONPUBLISH:";
    const uint32_t prefix_length = sizeof(prefix_str) - 1;

    if (buffer_length < prefix_length)
    {
        return false;
    }

    if (buffer_length == prefix_length)
    {
        if (ewfl_buffer_equals_buffer(buffer_ptr, prefix_str, prefix_length))
        {
            state_ptr->prefix_matches = true;
            return false;
        }
    }

    /* At this point the buffer it is longer than the prefix */

    /* We did not match the prefix in previous runs, just ignore the rest of the incoming characters */
    if (!state_ptr->prefix_matches)
    {
        return false;
    }
    else
    {
        /* This is for us, stop parsing other tokens further down the list */
        *stop_ptr = true;
    }

    /* At this point we have a matching prefix */

    /* Is the message complete? */
    if (buffer_ptr[buffer_length - 2] == '\r' && buffer_ptr[buffer_length - 1] == '\n')
    {
        /* Set the interface to URC mode */
        if (state_ptr->interface_ptr) state_ptr->interface_ptr->command_mode = false;

        /* Signal the match */
        return true;
    }
    else
    {
        /* Not yet matched */
        return false;
    }
}

static ewf_interface_tokenizer_pattern ewf_adapter_renesas_common_message_tokenizer_pattern4 =
{
    &ewf_adapter_renesas_common_message_tokenizer_pattern5,
    NULL,
    0,
    false,
    _ewf_adapter_renesas_common_message_tokenizer_pattern4_match_function,
    &ewf_adapter_renesas_common_message_tokenizer_pattern4_match_function_state
};

struct _ewf_adapter_renesas_common_message_tokenizer_pattern3_match_function_state
{
    ewf_interface* interface_ptr;
    bool prefix_matches;
};

static struct _ewf_adapter_renesas_common_message_tokenizer_pattern3_match_function_state ewf_adapter_renesas_common_message_tokenizer_pattern3_match_function_state = { 0 };

static bool _ewf_adapter_renesas_common_message_tokenizer_pattern3_match_function(const uint8_t* buffer_ptr, uint32_t buffer_length, const ewf_interface_tokenizer_pattern* pattern_ptr, bool* stop_ptr)
{
    if (!buffer_ptr) return false;
    if (!buffer_length) return false;
    if (!pattern_ptr) return false;
    if (!stop_ptr) return false;

    struct _ewf_adapter_renesas_common_message_tokenizer_pattern3_match_function_state* state_ptr =
        (struct _ewf_adapter_renesas_common_message_tokenizer_pattern3_match_function_state*)pattern_ptr->data_ptr;

    /* Initialize the state on a new buffer */
    if (buffer_length == 1)
    {
        state_ptr->prefix_matches = false;
        return false;
    }

    /* Add a NULL terminator - explicit const override */
    ((char*)buffer_ptr)[buffer_length] = 0;

    const char prefix_str[] = "\r\n+SQNSH:";
    const uint32_t prefix_length = sizeof(prefix_str) - 1;

    if (buffer_length < prefix_length)
    {
        return false;
    }

    if (buffer_length == prefix_length)
    {
        if (ewfl_buffer_equals_buffer(buffer_ptr, prefix_str, prefix_length))
        {
            state_ptr->prefix_matches = true;
            return false;
        }
    }

    /* At this point the buffer it is longer than the prefix */

    /* We did not match the prefix in previous runs, just ignore the rest of the incoming characters */
    if (!state_ptr->prefix_matches)
    {
        return false;
    }
    else
    {
        /* This is for us, stop parsing other tokens further down the list */
        *stop_ptr = true;
    }

    /* At this point we have a matching prefix */

    /* Is the message complete? */
    if (buffer_ptr[buffer_length - 2] == '\r' && buffer_ptr[buffer_length - 1] == '\n')
    {
        /* Set the interface to URC mode */
        if (state_ptr->interface_ptr) state_ptr->interface_ptr->command_mode = false;

        /* Signal the match */
        return true;
    }
    else
    {
        /* Not yet matched */
        return false;
    }
}

static ewf_interface_tokenizer_pattern ewf_adapter_renesas_common_message_tokenizer_pattern3 =
{
    &ewf_adapter_renesas_common_message_tokenizer_pattern4,
    NULL,
    0,
    false,
    _ewf_adapter_renesas_common_message_tokenizer_pattern3_match_function,
    &ewf_adapter_renesas_common_message_tokenizer_pattern3_match_function_state
};

struct _ewf_adapter_renesas_common_message_tokenizer_pattern2_match_function_state
{
    ewf_interface* interface_ptr;
    bool prefix_matches;
};

static struct _ewf_adapter_renesas_common_message_tokenizer_pattern2_match_function_state ewf_adapter_renesas_common_message_tokenizer_pattern2_match_function_state = { 0 };

static bool _ewf_adapter_renesas_common_message_tokenizer_pattern2_match_function(const uint8_t* buffer_ptr, uint32_t buffer_length, const ewf_interface_tokenizer_pattern* pattern_ptr, bool* stop_ptr)
{
    if (!buffer_ptr) return false;
    if (!buffer_length) return false;
    if (!pattern_ptr) return false;
    if (!stop_ptr) return false;

    struct _ewf_adapter_renesas_common_message_tokenizer_pattern2_match_function_state* state_ptr =
        (struct _ewf_adapter_renesas_common_message_tokenizer_pattern2_match_function_state*)pattern_ptr->data_ptr;

    /* Initialize the state on a new buffer */
    if (buffer_length == 1)
    {
        state_ptr->prefix_matches = false;
        return false;
    }

    /* Add a NULL terminator - explicit const override */
    ((char*)buffer_ptr)[buffer_length] = 0;

    /* Define the message prefixes and calculate their lengths */
     const char prefix1_str[] = "+SQNSRING: ";
     const uint32_t prefix1_length = sizeof(prefix1_str) - 1;
     const char prefix2_str[] = "\r\n+SQNSRING: ";
     const uint32_t prefix2_length = sizeof(prefix2_str) - 1;

     const uint32_t prefix_min_length = prefix1_length;
     const uint32_t prefix_max_length = prefix2_length;

     /* If the buffer is smaller than the prefix, then it is not yet for us */
     if (buffer_length < prefix_min_length)
     {
         return false;
     }


     /* We did not match the prefix in previous runs, try to match */
     if (!state_ptr->prefix_matches)
     {
         /* prefix1 */
         {
             /* If the buffer contains as many characters as the prefix, then look if it is for us */
             if (buffer_length == prefix1_length)
             {
                 if (ewfl_buffer_equals_buffer(buffer_ptr, prefix1_str, prefix1_length))
                 {
                     state_ptr->prefix_matches = true;
                     return false;
                 }
             }
         }

         /* prefix2 */
         {
             /* If the buffer contains as many characters as the prefix, then look if it is for us */
             if (buffer_length == prefix2_length)
             {
                 if (ewfl_buffer_equals_buffer(buffer_ptr, prefix2_str, prefix2_length))
                 {
                     state_ptr->prefix_matches = true;
                     return false;
                 }
             }
         }

         /* just ignore the rest of the incoming characters */
         return false;
     }

     /* If the buffer is bigger than the biggest prefix, and we do not have a match yet, then it is not yet for us */
     if (buffer_length > prefix_max_length && !state_ptr->prefix_matches)
     {
         return false;
     }

     /* At this point the buffer it is longer than the prefix */

     if (state_ptr->prefix_matches)
     {
         /* This is for us, stop parsing other tokens further down the list */
         *stop_ptr = true;
     }

     /* At this point we have a matching prefix */

     /* Is the message complete? */
     if (buffer_ptr[buffer_length - 2] == '\r' && buffer_ptr[buffer_length - 1] == '\n')
     {
         /* Set the interface to URC mode */
         if (state_ptr->interface_ptr) state_ptr->interface_ptr->command_mode = false;

         /* Signal the match */
         return true;
     }
     else
     {
         /* Not yet matched */
         return false;
     }
 }

static ewf_interface_tokenizer_pattern ewf_adapter_renesas_common_message_tokenizer_pattern2 =
{
    &ewf_adapter_renesas_common_message_tokenizer_pattern3,
    NULL,
    0,
    false,
    _ewf_adapter_renesas_common_message_tokenizer_pattern2_match_function,
    &ewf_adapter_renesas_common_message_tokenizer_pattern2_match_function_state
};

struct _ewf_adapter_renesas_common_message_tokenizer_pattern1_match_function_state
{
    ewf_interface* interface_ptr;
    bool prefix_matches;
};

static struct _ewf_adapter_renesas_common_message_tokenizer_pattern1_match_function_state ewf_adapter_renesas_common_message_tokenizer_pattern1_match_function_state = { 0 };

static bool _ewf_adapter_renesas_common_message_tokenizer_pattern1_match_function(const uint8_t* buffer_ptr, uint32_t buffer_length, const ewf_interface_tokenizer_pattern* pattern_ptr, bool* stop_ptr)
{
    if (!buffer_ptr) return false;
    if (!buffer_length) return false;
    if (!pattern_ptr) return false;
    if (!stop_ptr) return false;

    struct _ewf_adapter_renesas_common_message_tokenizer_pattern1_match_function_state* state_ptr =
        (struct _ewf_adapter_renesas_common_message_tokenizer_pattern1_match_function_state*)pattern_ptr->data_ptr;

    /* Initialize the state on a new buffer */
    if (buffer_length == 1)
    {
        state_ptr->prefix_matches = false;
        return false;
    }

    /* Add a NULL terminator - explicit const override */
    ((char*)buffer_ptr)[buffer_length] = 0;

    const char prefix_str[] = "\r\n+SQNSMQTTONMESSAGE:";
    const uint32_t prefix_length = sizeof(prefix_str) - 1;

    if (buffer_length < prefix_length)
    {
        return false;
    }

    if (buffer_length == prefix_length)
    {
        if (ewfl_buffer_equals_buffer(buffer_ptr, prefix_str, prefix_length))
        {
            state_ptr->prefix_matches = true;
            return false;
        }
    }

    /* At this point the buffer it is longer than the prefix */

    /* We did not match the prefix in previous runs, just ignore the rest of the incoming characters */
    if (!state_ptr->prefix_matches)
    {
        return false;
    }
    else
    {
        /* This is for us, stop parsing other tokens further down the list */
        *stop_ptr = true;
    }

    /* At this point we have a matching prefix */

    /* Is the message complete? */
    if (buffer_ptr[buffer_length - 2] == '\r' && buffer_ptr[buffer_length - 1] == '\n')
    {
        /* Set the interface to URC mode */
        if (state_ptr->interface_ptr) state_ptr->interface_ptr->command_mode = false;

        /* Signal the match */
        return true;
    }
    else
    {
        /* Not yet matched */
        return false;
    }
}

static ewf_interface_tokenizer_pattern ewf_adapter_renesas_common_message_tokenizer_pattern1 =
{
    &ewf_adapter_renesas_common_message_tokenizer_pattern2,
    NULL,
    0,
    false,
    _ewf_adapter_renesas_common_message_tokenizer_pattern1_match_function,
    &ewf_adapter_renesas_common_message_tokenizer_pattern1_match_function_state
};

static ewf_interface_tokenizer_pattern* ewf_adapter_renesas_common_message_tokenizer_pattern_ptr = &ewf_adapter_renesas_common_message_tokenizer_pattern1;

ewf_result ewf_adapter_renesas_common_start(ewf_adapter* adapter_ptr)
{
    EWF_ADAPTER_VALIDATE_POINTER(adapter_ptr);
    ewf_adapter_renesas_common* adapter_renesas_common_ptr = (ewf_adapter_renesas_common*)adapter_ptr->implementation_ptr;
    ewf_interface* interface_ptr = adapter_ptr->interface_ptr;
    EWF_INTERFACE_VALIDATE_POINTER(interface_ptr);

    ewf_result result;

    /* Initialize the interface */
    if (ewf_result_failed(result = ewf_interface_init(interface_ptr)))
    {
        EWF_LOG_ERROR("Failed to initialize the interface: ewf_result %d.\n", result);
        return EWF_RESULT_INTERFACE_INITIALIZATION_FAILED;
    }

    /* Set the interface default timeout */
    if (ewf_result_failed(result = ewf_interface_default_timeout_set(interface_ptr, EWF_PLATFORM_TICKS_PER_SECOND * 5)))
    {
        EWF_LOG_ERROR("Failed to set the interface default timeout: ewf_result %d.\n", result);
        return EWF_RESULT_INTERFACE_INITIALIZATION_FAILED;
    }

    /* Initialize the interface tokenizer patterns */

    ewf_adapter_renesas_common_message_tokenizer_pattern1_match_function_state.interface_ptr = interface_ptr;
    ewf_adapter_renesas_common_message_tokenizer_pattern2_match_function_state.interface_ptr = interface_ptr;
    ewf_adapter_renesas_common_message_tokenizer_pattern3_match_function_state.interface_ptr = interface_ptr;
    ewf_adapter_renesas_common_message_tokenizer_pattern4_match_function_state.interface_ptr = interface_ptr;
    ewf_adapter_renesas_common_message_tokenizer_pattern5_match_function_state.interface_ptr = interface_ptr;
    ewf_adapter_renesas_common_message_tokenizer_pattern6_match_function_state.interface_ptr = interface_ptr;
    ewf_adapter_renesas_common_message_tokenizer_pattern7_match_function_state.interface_ptr = interface_ptr;
    if (ewf_result_failed(result = ewf_interface_tokenizer_message_pattern_set(interface_ptr, ewf_adapter_renesas_common_message_tokenizer_pattern_ptr)))
    {
        EWF_LOG_ERROR("Failed to set the interface message tokenizer pattern: ewf_result %d.\n", result);
        return EWF_RESULT_INTERFACE_INITIALIZATION_FAILED;
    }

    if (ewf_result_failed(result = ewf_interface_tokenizer_command_response_end_pattern_set(interface_ptr, ewf_adapter_renesas_common_command_response_end_tokenizer_pattern_ptr)))
    {
        EWF_LOG_ERROR("Failed to set the interface command response end tokenizer pattern: ewf_result %d.\n", result);
        return EWF_RESULT_INTERFACE_INITIALIZATION_FAILED;
    }

    if (ewf_result_failed(result = ewf_interface_tokenizer_urc_pattern_set(interface_ptr, ewf_adapter_renesas_common_urc_tokenizer_pattern_ptr)))
    {
        EWF_LOG_ERROR("Failed to set the interface URC tokenizer pattern: ewf_result %d.\n", result);
        return EWF_RESULT_INTERFACE_INITIALIZATION_FAILED;
    }

    /* Start the interface */
    if (ewf_result_failed(result = ewf_interface_start(interface_ptr)))
    {
        EWF_LOG_ERROR("Failed to start the interface: ewf_result %d.\n", result);
        return EWF_RESULT_INTERFACE_INITIALIZATION_FAILED;
    }

    /* AT - wake the modem */
    if (ewf_result_failed(result = ewf_interface_send_command(interface_ptr, "AT\r"))) return result;
    if (ewf_result_failed(result = ewf_interface_drop_response(interface_ptr))) return result;
    if (ewf_result_failed(result = ewf_interface_send_command(interface_ptr, "AT\r"))) return result;
    if (ewf_result_failed(result = ewf_interface_drop_response(interface_ptr))) return result;
    if (ewf_result_failed(result = ewf_interface_send_command(interface_ptr, "AT\r"))) return result;
    if (ewf_result_failed(result = ewf_interface_drop_response(interface_ptr))) return result;

    /* Disable local echo */
    if (ewf_result_failed(result = ewf_interface_send_command(interface_ptr, "ATE0\r"))) return result;
    if (ewf_result_failed(result = ewf_interface_drop_response(interface_ptr))) return result;

    /* Check that the modem is responding */
    if (ewf_result_failed(result = ewf_interface_send_command(interface_ptr, "AT\r"))) return result;
    if (ewf_result_failed(result = ewf_interface_verify_response(interface_ptr, "\r\nOK\r\n"))) return result;

    /* Set the error Message Format */
    if (ewf_result_failed(result = ewf_interface_send_command(interface_ptr, "AT+CMEE=1\r"))) return result;
    if (ewf_result_failed(result = ewf_interface_verify_response(interface_ptr, "\r\nOK\r\n"))) return result;

#if EWF_ADAPTER_RENESAS_COMMON_TCP_ENABLED || EWF_ADAPTER_RENESAS_COMMON_UDP_ENABLED
    if (ewf_result_failed(result = ewf_adapter_renesas_common_internet_start(adapter_ptr)))
    {
        EWF_LOG_ERROR("Failed to start the internet API: ewf_result %d.\n", result);
        return EWF_RESULT_INTERFACE_INITIALIZATION_FAILED;
    }
#endif

#if EWF_ADAPTER_RENESAS_COMMON_TLS_BASIC_ENABLED
    if (ewf_result_failed(result = ewf_adapter_renesas_common_tls_basic_start(adapter_ptr)))
    {
        EWF_LOG_ERROR("Failed to start the TLS basic API: ewf_result %d.\n", result);
        return EWF_RESULT_INTERFACE_INITIALIZATION_FAILED;
    }
#endif

#if EWF_ADAPTER_RENESAS_COMMON_MQTT_BASIC_ENABLED
    if (ewf_result_failed(result = ewf_adapter_renesas_common_mqtt_basic_start(adapter_ptr)))
    {
        EWF_LOG_ERROR("Failed to start the MQTT basic API: ewf_result %d.\n", result);
        return EWF_RESULT_INTERFACE_INITIALIZATION_FAILED;
    }
#endif
    adapter_renesas_common_ptr->module_start = false;

    /* All ok! */
    return EWF_RESULT_OK;
}

ewf_result ewf_adapter_renesas_common_stop(ewf_adapter* adapter_ptr)
{
    EWF_ADAPTER_VALIDATE_POINTER(adapter_ptr);
    ewf_interface* interface_ptr = adapter_ptr->interface_ptr;
    EWF_INTERFACE_VALIDATE_POINTER(interface_ptr);

    ewf_result result;

    /* Stop the APIs */

#if EWF_ADAPTER_RENESAS_COMMON_NVM_ENABLED
    if (ewf_result_failed(result = ewf_adapter_renesas_common_nvm_stop(adapter_ptr)))
    {
        EWF_LOG_ERROR("Failed to start the NVM API: ewf_result %d.\n", result);
        return EWF_RESULT_INTERFACE_INITIALIZATION_FAILED;
    }
#endif

#if defined(EWF_ADAPTER_RENESAS_COMMON_TCP_ENABLED) || defined(EWF_ADAPTER_RENESAS_COMMON_UDP_ENABLED)
    if (ewf_result_failed(result = ewf_adapter_renesas_common_internet_stop(adapter_ptr)))
    {
        EWF_LOG_ERROR("Failed to stop the internet API: ewf_result %d.\n", result);
        return EWF_RESULT_INTERFACE_INITIALIZATION_FAILED;
    }
#endif

#if EWF_ADAPTER_RENESAS_COMMON_TLS_BASIC_ENABLED
    if (ewf_result_failed(result = ewf_adapter_renesas_common_tls_basic_stop(adapter_ptr)))
    {
        EWF_LOG_ERROR("Failed to stop the TLS basic API: ewf_result %d.\n", result);
        return EWF_RESULT_INTERFACE_INITIALIZATION_FAILED;
    }
#endif

#if EWF_ADAPTER_RENESAS_COMMON_MQTT_BASIC_ENABLED
    if (ewf_result_failed(result = ewf_adapter_renesas_common_mqtt_basic_stop(adapter_ptr)))
    {
        EWF_LOG_ERROR("Failed to stop the MQTT basic API: ewf_result %d.\n", result);
        return EWF_RESULT_INTERFACE_INITIALIZATION_FAILED;
    }
#endif

    /* Stop the interface */
    if (ewf_result_failed(result = ewf_interface_stop(interface_ptr)))
    {
        EWF_LOG_ERROR("Failed to clean the interface: ewf_result %d.\n", result);
        return EWF_RESULT_INTERFACE_INITIALIZATION_FAILED;
    }

    /* Cleanup the interface */
    if (ewf_result_failed(result = ewf_interface_init(interface_ptr)))
    {
        EWF_LOG_ERROR("Failed to clean the interface: ewf_result %d.\n", result);
        return EWF_RESULT_INTERFACE_INITIALIZATION_FAILED;
    }
    return EWF_RESULT_OK;
}
