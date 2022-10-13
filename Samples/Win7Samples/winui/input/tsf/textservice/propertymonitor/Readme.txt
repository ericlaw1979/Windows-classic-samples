https://learn.microsoft.com/en-us/windows/win32/tsf/why-use-text-services-framework
https://learn.microsoft.com/en-us/windows/win32/tsf/architecture


The term "application" refers to a TSF-enabled application, the term "text service" refers to a TSF text service,
and the term "manager" refers to the TSF manager.

TSF-enabled applications can receive text input from any text service that supports TSF without having to be aware of any details of the text source.

Text services supply text to applications in a way that does not require any detailed knowledge of the applications that produce or receive the text. For example, a text service can provide text input from speech or handwriting input devices.

A text service functions as a text provider to an application. A text service can obtain text from, and write text to, an application.
A text service can also associate data and properties with a block of text. A text service is implemented as a COM in-proc server that registers itself
with TSF. When registered, the user interacts with the text service using the language bar or keyboard shortcuts. Multiple text services can be installed.
