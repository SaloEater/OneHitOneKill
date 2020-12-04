////////////////////////////////////////////////////////////////////////////////////////////////////////////
// библиотека patcher_x86.dll 
// распространяется свободно(бесплатно)
// авторское право: Баринов Александр (baratorch), e-mail: baratorch@yandex.ru
// форма реализации низкоуровневых хуков (LoHook) отчасти позаимствована у Berserker (из ERA)
// ENG
// library patcher_x86.dll 
// distributed free of charge
// author: Barinov Alexander (baratorch), e-mail: baratorch@yandex.ru
// the design of low-level hooks (LoHook) partially got from Berserker's ERA
////////////////////////////////////////////////////////////////////////////////////////////////////////////

// версия 4.2
// ENG
// version 4.2

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ОПИСАНИЕ.
//
// ! библиотека предоставляет:
//  - удобные унифицированные централизованные 
//    инструменты для установки патчей и хуков
//    в код целевой программы.
//  - дополнительные инструменты: дизассемблер длин опкодов и функцию
//    копирующую код с корректным переносом опкодов jmp и call c 
//    относительной адресацией
// ! библиотека позволяет
//  - устанавливать как простые так и сложные патчи.
//  - устанавливать высокоуровневые хуки, замещая оригинальные функции в
//    в целевом коде на свои, не заботясь о регистрах процессора,
//    стеке, и возврате в оригинальный код.
//  - устанавливать высокоуровневые хуки один на другой
//    не исключая а дополняя при этом функциональность хуков
//    установленных раньше последнего
//  - устанавливать низкоуровневые хуки с высокоуровневым доступом к
//    регистрам процессора, стэку, затертому коду и адресу возврата в код
//  - отменять любой патч и хук, установленный с помощью этой библиотеки.
//  - узнать задействован ли определенный мод, использующий библиотеку
//  - узнать какой мод (использующий библиотеку) установил определенный патч/хук
//  - получить полный доступ ко всем патчам/хукам, установленным из других модов 
//    с помощью этой библиотеки
//  - легко и быстро обнаружить конфликтующие патчи из разных модов
//    (использующих эту библиотеку) 1) отмечая в логе такие конфликты как:
//        - устанавливаются патчи/хуки разного размера на один адрес
//        - устанавливаются патчи/хуки перекрывающие один другого со смещением
//        - устанавливаются патчи поверх хуков и наоборот
//    а так же 2) давая возможность посмотреть дамп (общий листинг) всех патчей 
//    и хуков установленных с помощью этой библиотеки в конкретный момент времени.
// ENG
// DESCRIPTION.
//
// ! library provides:
//  - convenient unified centralized 
//    tools for patch and hook placing
//    into target program code.
//  - additional tools: opcode length disassembler and the function
//    that copies code correctly moving of jmp and call opcodes with 
//    relative addressing
// ! library allows
//  - place both simple and complex patches.
//  - place high-level hooks, replacing original functions
//    in target code to new ones, without bothering about CPU registers,
//    stack, and return to original code.
//  - place high-level hooks one above another
//    without removing previous hook functionality,
//    and with adding new functionality over it
//  - place low-level hooks with high-level access to
//    CPU registers, stack, replaced code and address of returning to original code
//  - cancel any patch and hook placed with this library.
//  - get to know if the specific mod that use this library is active
//  - get to know which mod (among ones using this library) placed the specific patch/hook
//  - get the full access to all patches/hooks, placed by other mods
//    with this library
//  - simply and quickly detect conflicting patches from different mods
//    (among ones using this library) 1) logging such conflicts as:
//        - patches/hooks of different size placed to the single address
//        - patches/hooks overwrote one another with shift
//        - patches are placed above hooks and vice versa
//    also 2) giving possibility of watching dump (overall listing) of all patches
//    and hooks placed with this library at by specific moment.
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ЛОГИРОВАНИЕ.
//
// по умолчанию в patcher_x86.dll логирование отключено, чтобы включить его,
// необходимо в той же папке создать файл patcher_x86.ini c единственной
// записью: Logging = 1 (Logging = 0 - отключает логирование снова)
// ENG
// LOGGING.
//
// by default logging in patcher_x86.dll is off, to turn it on
// you need to create file patcher_x86.ini in the folder of library with the single
//  record: Logging = 1 (Logging = 0 - disable logging again)
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ПРАВИЛА ИСПОЛЬЗОВАНИЯ.
//
// 1) каждый мод должен 1 раз вызвать функцию GetPatcher(), сохранив результат
//  например: Patcher* _P = GetPatcher();
// 2) затем с помощью метода Pather::CreateInstance нужно создать  
// экземпляр PatсherInstance со своим уникальным именем
//  например: PatсherInstance* _PI = _P->CreateInstance("MyMod");
// 3)  затем использовать методы классов Patсher и PatсherInstance
// непосредственно для работы с патчами и хуками
// ENG
// HOW TO USE.
//
// 1) each mod must call the function GetPatcher() once and save its result
//  example: Patcher* _P = GetPatcher();
// 2) then you need to create instance of PatсherInstance with unique name
// with Pather::CreateInstance method
//  example: PatсherInstance* _PI = _P->CreateInstance("MyMod");
// 3)  then use Patсher and PatсherInstance methods
// directly to work with patches and hooks
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#define _byte_ unsigned __int8
#define _word_ unsigned __int16
#define _dword_ unsigned __int32


//макросы CALL_? позволяют вызывать произвольную функцию по определенному адресу
//используются в том числе для вызова функций
//полученных с помощью HiHook::GetDefaultFunc и HiHook::GetOriginalFunc
// ENG
//CALL_? macros allow to call an arbitrary function at specific address
//their use includes the case of calling functions
//that are got with HiHook::GetDefaultFunc and HiHook::GetOriginalFunc
#define CALL_0(return_type, call_type, address) \
 ((return_type (call_type *)(void))address)()
#define CALL_1(return_type, call_type, address, a1) \
 ((return_type (call_type *)(_dword_))(address))((_dword_)(a1))
#define CALL_2(return_type, call_type, address, a1, a2) \
 ((return_type (call_type *)(_dword_,_dword_))(address))((_dword_)(a1),(_dword_)(a2))
#define CALL_3(return_type, call_type, address, a1, a2, a3) \
 ((return_type (call_type *)(_dword_,_dword_,_dword_))(address))((_dword_)(a1),(_dword_)(a2),(_dword_)(a3))
#define CALL_4(return_type, call_type, address, a1, a2, a3, a4) \
 ((return_type (call_type *)(_dword_,_dword_,_dword_,_dword_))(address))((_dword_)(a1),(_dword_)(a2),(_dword_)(a3),(_dword_)(a4))
#define CALL_5(return_type, call_type, address, a1, a2, a3, a4, a5) \
 ((return_type (call_type *)(_dword_,_dword_,_dword_,_dword_,_dword_))(address))((_dword_)(a1),(_dword_)(a2),(_dword_)(a3),(_dword_)(a4),(_dword_)(a5))
#define CALL_6(return_type, call_type, address, a1, a2, a3, a4, a5, a6) \
 ((return_type (call_type *)(_dword_,_dword_,_dword_,_dword_,_dword_,_dword_))(address))((_dword_)(a1),(_dword_)(a2),(_dword_)(a3),(_dword_)(a4),(_dword_)(a5),(_dword_)(a6))
#define CALL_7(return_type, call_type, address, a1, a2, a3, a4, a5, a6, a7) \
 ((return_type (call_type *)(_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_))(address))((_dword_)(a1),(_dword_)(a2),(_dword_)(a3),(_dword_)(a4),(_dword_)(a5),(_dword_)(a6),(_dword_)(a7))
#define CALL_8(return_type, call_type, address, a1, a2, a3, a4, a5, a6, a7, a8) \
 ((return_type (call_type *)(_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_))(address))((_dword_)(a1),(_dword_)(a2),(_dword_)(a3),(_dword_)(a4),(_dword_)(a5),(_dword_)(a6),(_dword_)(a7),(_dword_)(a8))
#define CALL_9(return_type, call_type, address, a1, a2, a3, a4, a5, a6, a7, a8, a9) \
 ((return_type (call_type *)(_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_))(address))((_dword_)(a1),(_dword_)(a2),(_dword_)(a3),(_dword_)(a4),(_dword_)(a5),(_dword_)(a6),(_dword_)(a7),(_dword_)(a8),(_dword_)(a9))
#define CALL_10(return_type, call_type, address, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) \
 ((return_type (call_type *)(_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_))(address))((_dword_)(a1),(_dword_)(a2),(_dword_)(a3),(_dword_)(a4),(_dword_)(a5),(_dword_)(a6),(_dword_)(a7),(_dword_)(a8),(_dword_)(a9),(_dword_)(a10))
#define CALL_11(return_type, call_type, address, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) \
 ((return_type (call_type *)(_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_))(address))((_dword_)(a1),(_dword_)(a2),(_dword_)(a3),(_dword_)(a4),(_dword_)(a5),(_dword_)(a6),(_dword_)(a7),(_dword_)(a8),(_dword_)(a9),(_dword_)(a10),(_dword_)(a11))
#define CALL_12(return_type, call_type, address, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) \
 ((return_type (call_type *)(_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_))(address))((_dword_)(a1),(_dword_)(a2),(_dword_)(a3),(_dword_)(a4),(_dword_)(a5),(_dword_)(a6),(_dword_)(a7),(_dword_)(a8),(_dword_)(a9),(_dword_)(a10),(_dword_)(a11),(_dword_)(a12))
#define CALL_13(return_type, call_type, address, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) \
 ((return_type (call_type *)(_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_))(address))((_dword_)(a1),(_dword_)(a2),(_dword_)(a3),(_dword_)(a4),(_dword_)(a5),(_dword_)(a6),(_dword_)(a7),(_dword_)(a8),(_dword_)(a9),(_dword_)(a10),(_dword_)(a11),(_dword_)(a12),(_dword_)(a13))
#define CALL_14(return_type, call_type, address, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) \
 ((return_type (call_type *)(_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_))(address))((_dword_)(a1),(_dword_)(a2),(_dword_)(a3),(_dword_)(a4),(_dword_)(a5),(_dword_)(a6),(_dword_)(a7),(_dword_)(a8),(_dword_)(a9),(_dword_)(a10),(_dword_)(a11),(_dword_)(a12),(_dword_)(a13),(_dword_)(a14))
#define CALL_15(return_type, call_type, address, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) \
 ((return_type (call_type *)(_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_))(address))((_dword_)(a1),(_dword_)(a2),(_dword_)(a3),(_dword_)(a4),(_dword_)(a5),(_dword_)(a6),(_dword_)(a7),(_dword_)(a8),(_dword_)(a9),(_dword_)(a10),(_dword_)(a11),(_dword_)(a12),(_dword_)(a13),(_dword_)(a14),(_dword_)(a15))
#define CALL_16(return_type, call_type, address, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) \
 ((return_type (call_type *)(_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_))(address))((_dword_)(a1),(_dword_)(a2),(_dword_)(a3),(_dword_)(a4),(_dword_)(a5),(_dword_)(a6),(_dword_)(a7),(_dword_)(a8),(_dword_)(a9),(_dword_)(a10),(_dword_)(a11),(_dword_)(a12),(_dword_)(a13),(_dword_)(a14),(_dword_)(a15),(_dword_)(a16))
#define CALL_17(return_type, call_type, address, a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17) \
 ((return_type (call_type *)(_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_,_dword_))(address))((_dword_)(a1),(_dword_)(a2),(_dword_)(a3),(_dword_)(a4),(_dword_)(a5),(_dword_)(a6),(_dword_)(a7),(_dword_)(a8),(_dword_)(a9),(_dword_)(a10),(_dword_)(a11),(_dword_)(a12),(_dword_)(a13),(_dword_)(a14),(_dword_)(a15),(_dword_)(a16),(_dword_)(a17))

#define CALL_VA(return_type, adress, a1, ...) \
 ((return_type (__cdecl *)(_dword_, ...))(adress))((_dword_)(a1), __VA_ARGS__)


// _bool_ - 4-х байтовый логический тип, как BOOL в Win32 API
// если очень захочется, можно заменить на BOOL или однобайтовый bool например
// ENG
// _bool_ - 4-byte boolean type, as BOOL in Win32 API
// if you want this much you may replace it for example with BOOL or 1-byte bool
#define _bool_ __int32

// все адреса и часть указателей определены этим типом,
// если вам удобнее по-другому, можете заменить _ptr_ на любой другой тип, void* или int например
// ENG
// all addresses and the part of pointers are defined with this type,
// if another way is more convenient for you, you may replace _ptr_ with any other type, for example void* or int
typedef _dword_ _ptr_;



// во всех структурах и интерфейсах библиотеки должно быть выравнивание - 4 байта
// ENG
// all the structures of library interface must have 4-byte alignment
#pragma pack(push, 4)


//тип "переменная", используется для возвращаемых методами Patcher::VarInit и Patcher::VarFind значений.
// ENG
//"variable" type, it is used for values which are returned by Patcher::VarInit and Patcher::VarFind methods.
class Variable
{
public:
	// возвращает значение 'переменной' (потокобезопасное обращение)
	// ENG
	// returns the value of 'variable' (thread-safe reference)
	virtual _dword_ __stdcall GetValue() = 0;
	// устанавливает значение 'переменной' (потокобезопасное обращение)
	// ENG
	// sets the value of 'variable' (thread-safe reference)
	virtual void __stdcall SetValue(_dword_ value) = 0;
	// возвращает указатель на значение (обращение к значению через указатель непотокобезопасно)
	// ENG
	// returns a pointer to the value (reference to the value via this pointer is not thread-safe)
	virtual _dword_* __stdcall GetPValue() = 0;
};

// тип 'регистр флагов', размер - 32 бита 
// используется в HookContext
// ENG
// 'flag register' type, size is 32 bits
// it is used it HookContext
struct FlagsRegister
{
	_dword_ CF : 1; //0
	_dword_ reserved_1 : 1; //1
	_dword_ PF : 1; //2
	_dword_ reserved_3 : 1; //3
	_dword_ AF : 1; //4
	_dword_ reserved_5 : 1; //5
	_dword_ ZF : 1; //6
	_dword_ SF : 1; //7
	_dword_ TF : 1; //8
	_dword_ IF : 1; //9
	_dword_ DF : 1; //10
	_dword_ OF : 1; //11
	_dword_ IOPL : 2; //12-13
	_dword_ NT : 1; //14
	_dword_ reserved_15 : 1; //15
	_dword_ RF : 1; //16
	_dword_ VM : 1; //17
	_dword_ AC : 1; //18
	_dword_ VIF : 1; //19
	_dword_ VIP : 1; //20
	_dword_ ID : 1; //21
	_dword_ reserved_22 : 1; //22
	_dword_ reserved_23 : 1; //23
	_dword_ reserved_24 : 1; //24
	_dword_ reserved_25 : 1; //25
	_dword_ reserved_26 : 1; //26
	_dword_ reserved_27 : 1; //27
	_dword_ reserved_28 : 1; //28
	_dword_ reserved_29 : 1; //29
	_dword_ reserved_30 : 1; //30
	_dword_ reserved_31 : 1; //31
};

// Структура HookContext
// используется в функциях сработавших по LoHook хуку
// ENG
// HookContext structure
// it is used in functions triggered by LoHook hook
struct HookContext
{
	int eax; //регистр EAX, чтение/изменение; ENG: EAX register, read/modify
	int ecx; //регистр ECX, чтение/изменение; ENG: ECX register, read/modify
	int edx; //регистр EDX, чтение/изменение; ENG: EDX register, read/modify
	int ebx; //регистр EBX, чтение/изменение; ENG: EBX register, read/modify
	int esp; //регистр ESP, чтение/изменение; ENG: ESP register, read/modify
	int ebp; //регистр EBP, чтение/изменение; ENG: EBP register, read/modify
	int esi; //регистр ESI, чтение/изменение; ENG: ESI register, read/modify
	int edi; //регистр EDI, чтение/изменение; ENG: EDI register, read/modify

	_ptr_ return_address; //адрес возврата, чтение/изменение; ENG: return address, read/modify

	FlagsRegister flags; //регистр флагов, чтение/изменение; ENG: flag register, read/modify
	// для языков программирования не поддерживающих битовые поля (напр. delphi)
	// flags может быть определен как _dword_ типа.
	// ENG
	// for programming languages that does not support bit field (i. e. delphi)
	// flags can be defined with _dword_ type.


	// функция Push имеет аналогичное действие команде процессора PUSH для контекста LoHook хука
	// при использовании с контекстом хука установленного с помощью WriteLoHook или CreateLoHook
	// размер памяти которая может быть помещена в стек с помощью этой функции ограничен 128 байтами.
	// при использовании с контекстом хука установленного с помощью WriteLoHookEx или CreateLoHookEx
	// этот размер устанавливается произвольно при вызове WriteLoHookEx или CreateLoHookEx.
	// ENG
	// Push function has the similar effect as CPU command PUSH for LoHook hook context
	// while using with context of hook placed by WriteLoHook or CreateLoHook
	// the overall size of data that can be added to the stack via this function is limited as 128 bytes.
	// while using with context of hook placed by WriteLoHookEx or CreateLoHookEx
	// this size is arbitrary set at WriteLoHookEx or CreateLoHookEx call.
	inline void Push(int v)
	{
		esp -= 4;
		*(int*)(esp) = v;
	}

	// функция Pop имеет аналогичное действие команде процессора POP для контекста LoHook хука
	// ENG
	// Pop function has the similar effect as CPU command POP for LoHook hook context
	inline int Pop()
	{
		int r = *(int*)(esp);
		esp += 4;
		return r;
	}
};



// значения возвращаемые функцией срабатываемой по LoHook хуку
// ENG
// values that may be returned by the function triggered by LoHook hook
#define EXEC_DEFAULT 1
#define NO_EXEC_DEFAULT 0
#define SKIP_DEFAULT 0


// значения возвращаемые Patch::GetType()
// ENG
// values that may be returned by Patch::GetType()
#define PATCH_  0
#define LOHOOK_ 1
#define HIHOOK_ 2


// значения передаваемые PatcherInstance::Write() и PatcherInstance::CreatePatch()
// ENG
// values that are transfered to PatcherInstance::Write() and PatcherInstance::CreatePatch()
#define DATA_ 0
#define CODE_ 1


// Абстрактный класс Patch
// создать экземпляр можно с
// помощью методов класса PatcherInstance
// ENG
// Abstract class Patch
// instance may be created
// with PatcherInstance class methods
class Patch
{
public:
	// возвращает адрес по которому устанавливается патч
	// ENG
	// returns the address on which patch is placed
	virtual _ptr_ __stdcall GetAddress() = 0;

	// возвращает размер патча
	// ENG
	// returns the size of the patch
	virtual _dword_ __stdcall GetSize() = 0;

	// возвращает уникальное имя экземпляра PatcherInstance, с помощью которого был создан патч
	// ENG
	// returns the unique name of PatcherInstance instance, which the patch was created via
	virtual char* __stdcall GetOwner() = 0;

	// возвращает тип патча
	// для не хука всегда PATCH_
	// для LoHook всегда LOHOOK_
	// для HiHook всегда HIHOOK_
	// ENG
	// returns the type of the patch
	// for not-hook it is always PATCH_
	// for LoHook it is always LOHOOK_
	// for HiHook it is always HIHOOK_
	virtual int  __stdcall GetType() = 0;

	// возвращает true, если патч применен и false, если нет.
	// ENG
	// returns true if the patch is applied and false if not
	virtual _bool_ __stdcall IsApplied() = 0;

	// применяет патч 
	// возвращает >= 0 , если патч/хук применился успешно
	// (возвращаемое значение является порядковым номером патча в последовательности
	// патчей, примененных по окрестности данного адреса, чем больше число, 
	// тем позднее был применен патч)
	// возвращает -2, если патч уже применен
	// Результат выполнения метода распространенно пишется в лог
	// ENG
	// applies the patch 
	// returns the value >= 0 if the patch/hook was applied successfully
	// (return value is the ordinal number of the patch in the sequence
	// of patches that are applied at (near) this address, the greater value,
	// the later the patch was applied)
	// returns -2 if the patch is already applied
	// The result of this method call is written to log in detail
	virtual _bool_ __stdcall Apply() = 0;

	// ApplyInsert применяет патч с указанием порядкового номера в
	// последовательности патчей, примененных по этому адресу.
	// возвращаемые значения аналогичны соответствующим в Patch::Apply
	// функции ApplyInsert можно аргументом передать значение, возвращаемое 
	// функцией Undo, чтобы применить патч в то же место, на котором тот был до отмены.
	// ENG
	// ApplyInsert applies the patch with the specific order in
	// the sequence of patches applied to this address.
	// return values are similar as Patch::Apply return values
	// you can give the value returned by Undo function to ApplyInsert 
	// to apply the patch at the place of the patch cancelled by Undo.
	virtual _bool_ __stdcall ApplyInsert(int zorder) = 0;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Метод Undo
	// Отменяет патч(хук) (в случае если патч применен последним - восстанавливает затертый код)
	// Возвращает число >= 0, если патч (хук) был отменен успешно 
	// (возвращаемое значение является номером патча в последовательности
	// патчей, примененных по данному адресу, чем больше число, 
	// тем позднее был применен патч)
	// Возвращает -2, если патч и так уже был отменен (не был применен)
	// Результат выполнения метода распространенно пишется в лог
	// ENG
	// Undo method
	// Cancels the patch (hook) (in case the patch was the last of the applied ones this method restores the replaced original code)
	// Returns the value >= 0, if the patch (hook) was cancelled successfully
	// (return value is the ordinal number of the patch in the sequence
	// of patches that are applied to this address, the greater value,
	// the later the patch was applied)
	// Returns -2, if the patch was already cancelled (or was not applied)
	// The result of this method call is written to log in detail
	virtual _bool_ __stdcall Undo() = 0;


	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Метод Destroy
	// Отменяет и безвозвратно уничтожает патч/хук
	// возвращает всегда 1 (для совместимости с более ранними версиями библиотеки)
	// Результат уничтожения распространенно пишется в лог
	// ENG
	// Destroy method
	// Cancels and irretrievably destroys th patch/hook
	// always returns 1 (for compatibility with для the earlier versions of the library)
	// The result of the destruction is written to log in detail
	virtual _bool_ __stdcall Destroy() = 0;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Метод GetAppliedBefore
	// возвращает патч примененный перед данным
	// возвращает NULL если данный патч применен первым
	// ENG
	// GetAppliedBefore method
	// returns the patch applied before this one
	// returns NULL if this patch was the first of applied ones
	virtual Patch* __stdcall GetAppliedBefore() = 0;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// Метод GetAppliedAfter
	// возвращает патч примененный после данного
	// возвращает NULL если данный патч применен последним
	// ENG
	// GetAppliedAfter method
	// returns the patch aplied after this one
	// returns NULL if this patch was the last of applied ones
	virtual Patch* __stdcall GetAppliedAfter() = 0;

};


// Абстрактный класс LoHook (унаследован от Patch, т.е. по сути лоу-хук является патчем)
// создать экземпляр можно с
// помощью методов класса PatcherInstance
// ENG
// Abstract class LoHook (the heir of Patch, so the LoHook is Patch in fact)
// instance may be created
// with PatcherInstance class methods
class LoHook : public Patch
{
};



// Тип функции, вызываемой низкоуровневым (LoHook) хуком.
// ENG
// Low-level hook (LoHook) triggered function type.
typedef int(__stdcall* _LoHookFunc_)(LoHook*, HookContext*);

// значения передаваемые как аргумент hooktype в PatcherInstance::WriteHiHook и PatcherInstance::CreateHiHook
// ENG
// the values for hooktype argument in PatcherInstance::WriteHiHook and PatcherInstance::CreateHiHook
#define CALL_ 0
#define SPLICE_ 1
#define FUNCPTR_ 2

// значения передаваемые как аргумент subtype в PatcherInstance::WriteHiHook и PatcherInstance::CreateHiHook
// ENG
// the values for subtype argument in PatcherInstance::WriteHiHook and PatcherInstance::CreateHiHook
#define DIRECT_  0
#define EXTENDED_ 1
#define SAFE_  2

// значения передаваемые как аргумент calltype в PatcherInstance::WriteHiHook и PatcherInstance::CreateHiHook
// the values for calltype argument in PatcherInstance::WriteHiHook and PatcherInstance::CreateHiHook
#define ANY_  0
#define STDCALL_ 0
#define THISCALL_ 1
#define FASTCALL_ 2 
#define CDECL_  3
#define FASTCALL_1 1 

// Абстрактный класс HiHook (унаследован от Patch, т.е. по сути хай-хук является патчем)
// создать экземпляр можно с помощью методов класса PatcherInstance
// ENG
// Abstract class HiHook (the heir of Patch, so the LoHook is Patch in fact)
// instance may be created with PatcherInstance class methods
class HiHook : public Patch
{
public:
	// возвращает указатель на функцию (на мост к функции в случае SPLICE_),
	// замещенную хуком
	// Внимание! Вызывая функцию для не примененного хука, можно получить
	// неактуальное (но рабочее) значение.
	// ENG
	// returns the pointer to the function (the bridge to the function in case of SPLICE_)
	// which is replaced with hook
	// Attention! Calling this function for hook which is not applied you can get
	// not relevant (but working) value
	virtual _ptr_ __stdcall GetDefaultFunc() = 0;

	// возвращает указатель на оригинальную функцию (на мост к функции в случае SPLICE_),
	// замещенную хуком (хуками) по данному адресу
	// (т.е. возвращает GetDefaultFunc() для первого примененного хука по данному адресу)
	// Внимание! Вызывая функцию для не примененного хука, можно получить
	// неактуальное (но рабочее) значение.
	// ENG
	// returns the pointer to the original function (the bridge to the function in case of SPLICE_)
	// which is replaced with hook (hooks) at this address
	// (i. e. returns GetDefaultFunc() for the first hook applied at this address)
	// Attention! Calling this function for hook which is not applied you can get
	// not relevant (but working) value
	virtual _ptr_ __stdcall GetOriginalFunc() = 0;

	// возвращает адрес возврата в оригинальный код
	// можно использовать внутри хук-функции
	// SPLICE_ EXTENDED_ или SAFE_ хука, чтобы узнать откуда она была вызвана
	// для SPLICE_ DIRECT_ хука функция возвращает всегда 0 (т.е. для DIRECT_ хука возможности узнать адрес возврата через нее - нет)
	// ENG
	// returns return to the original code address
	// can be used inside the hook-function
	// SPLICE_ EXTENDED_ or SAFE hook for getting know where it was called from
	// for SPLICE_ DIRECT_ hook tis function always returns 0 (i. e. there is no way for DIRECT_ hook to get to know the return address through it)
	virtual _ptr_ __stdcall GetReturnAddress() = 0;


	//# ver 2.1
	// ENG
	//# ver 2.1

	// устанавливает значение пользовательских данных хука
	// ENG
	// set the value of hook user data
	virtual void __stdcall SetUserData(_dword_ data) = 0;
	// возвращает значение пользовательских данных хука
	// если не задано пользователем, то равно 0
	// ENG
	// returns the value of hook user data
	// if the data was not set by user the function returns 0
	virtual _dword_ __stdcall GetUserData() = 0;
};




// Абстрактный класс PatcherInstance
// создать/получить экземпляр можно с помощью методов CreateInstance и GetInstance класса Patcher
// непосредственно позволяет создавать/устанавливать патчи и хуки в код,
// добавляя их в дерево всех патчей/хуков, созданных библиотекой patcher_x86.dll
// Abstract class PatcherInstance
// instance may be created/got with CreateInstance or GetInstance methods of Patcher class
// this class directly allows to create/apply patches and hooks to code,
// adding them to common tree of patches/hooks created with patcher_x86.dll library
class PatcherInstance
{
public:
	////////////////////////////////////////////////////////////
	// Метод WriteByte
	// пишет однобайтовое число по адресу address
	// (создает и применяет DATA_ патч)
	// Возвращает указатель на патч
	// ENG
	// WriteByte method
	// writes a 1-byte number at the specified address
	// (creates and applies DATA_ patch)
	// Returns a pointer to the patch
	virtual Patch* __stdcall WriteByte(_ptr_ address, int value) = 0;

	////////////////////////////////////////////////////////////
	// Метод WriteWord
	// пишет двухбайтовое число по адресу address
	// (создает и применяет DATA_ патч)
	// Возвращает указатель на патч
	// ENG
	// WriteWord method
	// writes a 2-byte number at the specified address
	// (creates and applies DATA_ patch)
	// Returns a pointer to the patch
	virtual Patch* __stdcall WriteWord(_ptr_ address, int value) = 0;

	////////////////////////////////////////////////////////////
	// Метод WriteDword
	// пишет четырехбайтовое число по адресу address
	// (создает и применяет DATA_ патч)
	// Возвращает указатель на патч
	// ENG
	// WriteDword method
	// writes a 4-byte number at the specified address
	// (creates and applies DATA_ patch)
	// Returns a pointer to the patch
	virtual Patch* __stdcall WriteDword(_ptr_ address, int value) = 0;

	////////////////////////////////////////////////////////////
	// Метод WriteJmp
	// пишет jmp to опкод по адресу address
	// (создает и применяет CODE_ патч)
	// Возвращает указатель на патч
	// патч закрывает целое количество опкодов,
	// т.е. размер патча >= 5, разница заполняется NOP'ами.
	// ENG
	// WriteJmp method
	// writes 'jmp to' opcode at the specified address
	// (creates and applies CODE_ patch)
	// Returns a pointer to the patch
	// patch replaces an integer number of opcodes,
	// i. e. if the size of the patch is >= 5 then the gap is filled with NOPs
	virtual Patch* __stdcall WriteJmp(_ptr_ address, _ptr_ to) = 0;

	////////////////////////////////////////////////////////////
	// Метод WriteHexPatch
	// пишет по адресу address последовательность байт,
	// определенную hex_str
	// (создает и применяет DATA_ патч)
	// Возвращает указатель на патч
	// hex_str - си-строка может содержать шестнадцатеричные цифры
	// 0123456789ABCDEF (только верхний регистр!) остальные символы 
	// при чтении методом hex_str игнорируются(пропускаются)
	// удобно использовать в качестве аргумента этого метода
	// скопированное с помощью Binary copy в OllyDbg
	/* пример:
	  pi->WriteHexPatch(0x57b521, "6A 01  6A 00");
	*/
	// ENG
	// WriteHexPatch method
	// writes the sequence of bytes defined by hex_str
	// at the specified address
	// (creates and applies DATA_ patch)
	// Returns a pointer to the patch
	// hex_str is C-style string, it can contain only hexadecimal digits
	// 0123456789ABCDEF (only uppercase!), other symbols
	// are ignored (skipped) when this method reads hex_str
	// it is convenient to get an argument for this method
	// by copying with 'Binary copy' in OllyDbg
	/* an example:
	  pi->WriteHexPatch(0x57b521, "6A 01  6A 00");
	*/
	virtual Patch* __stdcall WriteHexPatch(_ptr_ address, char* hex_str) = 0;

	////////////////////////////////////////////////////////////
	// Метод WriteCodePatchVA
	// в оригинальном виде применение метода не предполагается,
	// смотрите (ниже) описание метода-оболочки WriteCodePatch
	// ENG
	// WriteCodePatchVA method
	// this method is not supposed to be used in the original form
	// see the description of the shell method WriteCodePatch (below)
	virtual Patch* __stdcall WriteCodePatchVA(_ptr_ address, char* format, _dword_* va_args) = 0;

	////////////////////////////////////////////////////////////
	// Метод WriteLoHook
	// создает по адресу address низкоуровневый хук (CODE_ патч) и применяет его
	// возвращает указатель на хук
	// func - функция вызываемая при срабатывании хука
	// должна иметь тип _LoHookFunc_: int __stdcall func(LoHook* h, HookContext* c);
	// в HookContext* c передаются для чтения/изменения 
	// регистры процессора и адрес возврата
	// если func возвращает EXEC_DEFAULT, то 
	// после завершения func выполняется затертый хуком код.
	// если - SKIP_DEFAULT - затертый код не выполняется
	//
	// ВНИМАНИЕ! 
	// размер памяти, которая может быть помещена в стек контекста
	// с помощью использования c->esp и с->Push, ограничен 128 байтами.
	// если требуется иное ограничение используйте метод WriteLoHookEx или CreateLoHookEx.
	// ENG
	// WriteLoHook method
	// creates a low-level hook (CODE_ patch) at the specified address and applies it
	// returns a pointer to the hook
	// func is the functions which is called when the hook is triggered
	// it must be of _LoHookFunc_ type: int __stdcall func(LoHook* h, HookContext* c);
	// HookContext* c allows to read/modify
	// CPU register values and return address
	// if func returns EXEC_DEFAULT
	// the original code replaced by hook is executed after finishing func call
	// in func returns SKIP_DEFAULT then the replaced code is not executed
	// 
	// ATTENTION!
	// the size of the memory that can be placed to stack of the context
	// with c->esp and c->Push cannot exceed 128 bytes.
	// if you need another limit, use WriteLoHookEx or CreateLoHookEx method.
	virtual LoHook* __stdcall WriteLoHook(_ptr_ address, _LoHookFunc_ func) = 0;

	////////////////////////////////////////////////////////////
	// Метод WriteHiHook
	// создает по адресу address высокоуровневый хук и применяет его
	// возвращает указатель на хук
	//
	// new_func - функция замещающая оригинальную
	//
	// hooktype - тип хука:
	//  CALL_ -  хук НА ВЫЗОВ функции по адресу address
	//     поддерживаются опкоды E8 и FF 15, в остальных случаях хук не устанавливается
	//     и в лог пишется информация об этой ошибке
	//  SPLICE_ - хук непосредственно НА САМУ ФУНКЦИЮ по адресу address
	//  FUNCPTR_ - хук на функцию в указателе (применяется редко, в основном для хуков в таблицах импорта)
	//
	// subtype - подтип хука:
	//  DIRECT_ - new_func имеет тот же вид что и
	//     оригинальная замещаемая функция
	//     примечание: вместо __thiscall f(this) 
	//        можно использовать __fastcall f(this_)
	//        вместо __thiscall f(this, ...)  можно использовать 
	//        __fastcall f(this_, no_used_edx, ...) 
	//  EXTENDED_ - функции new_func первым стековым аргументом передается
	//     указатель на экземпляр HiHook и, в случае 
	//     соглашений исходной функции __thiscall и __fastcall
	//     регистровые аргументы передаются стековыми вторыми 
	// Таким образом функция new_func должна иметь вид
	// ? __stdcall new_func(HiHook* hook, ?) для ? ? orig(?)
	// Для ? __cdecl orig(?) допускается также использование функций,
	// объявленных как ? __cdecl new_func(HiHook* hook, ?)
	// (для совместимости с более ранними версиями библиотеки)
	//
	// ВНИМАНИЕ! EXTENDED_ FASTCALL_ поддерживает только функции с 2-мя и более аргументами
	// для __fastcall c 1 аргументом используйте EXTENDED_ FASTCALL_1 / EXTENDED_ THISCALL_
	//
	//   SAFE_ - то же самое что и EXTENDED_, однако перед вызовом (на время вызова) GetDefaultFunc() восстанавливаются
	//    значения регистров процессора EAX, ECX (если не FASTCALL_ и не THISCALL_),
	//    EDX (если не FASTCALL_), EBX, ESI, EDI, бывшие на момент вызова замещенной функции
	//
	//  в подавляющем большинстве случаев удобнее использовать EXTENDED_
	//  но DIRECT_ выполняется быстрее из-за отсутствия моста к новой замещающей функции
	//
	// calltype - соглашение о вызове оригинальной замещаемой функции:
	//  STDCALL_
	//  THISCALL_
	//  FASTCALL_
	//  CDECL_
	// необходимо верно указывать соглашение для того чтобы EXTENDED_ хук правильно
	// построил мост к новой замещающей функции
	//
	// CALL_, SPLICE_ хук является CODE_ патчем
	// FUNCPTR_ хук является DATA_ патчем
	//
	// ENG
	// WriteHiHook method
	// creates a high-level hook at the specified address and applies it
	// returns a pointer to the hook
	//
	// new_func is a function which replaces the original one
	//
	// hooktype is the type of the hook:
	//  CALL_ - hook applied ON CALL of the function from the specified address
	//     opcodes E8 and FF 15 are supported, in other cases hook is not applied
	//     and information about this error is written to log
	//  SPLICE_ - hook is applied directly ON FUNCTION ITSELF at the specified address
	//  FUNCPTR_ - hook is applied on the pointer to the function (rarely used, mostly for hooks in import tables)
	//
	// subtype is hook subtype:
	//  DIRECT_ - new_func has the same type as
	//     the replaced original function
	//     note: instead of __thiscall f(this) 
	//        you can use __fastcall f(this_)
	//        instead of __thiscall f(this, ...)  you can use 
	//        __fastcall f(this_, no_used_edx, ...) 
	//  EXTENDED_ - the pointer to HiHook instance is passed to new_func function
	//     as the first stack argument and in case 
	//     the original function was of __thiscall or __fastcall calling convention
	//     register arguments are passed as the second stack arguments
	// So, new_func function must take the form
	// ? __stdcall new_func(HiHook* hook, ?) for ? ? orig(?)
	// For ? __cdecl orig(?) you can also use the function,
	// declared as ? __cdecl new_func(HiHook* hook, ?)
	// (for the compatibility with the previous versions of the library)
	//
	// ATTENTION! EXTENDED_ FASTCALL_ supports only the functions with 2 or more arguments
	// for __fastcall with 1 argument use EXTENDED_ FASTCALL_1 / EXTENDED_ THISCALL_
	//
	//   SAFE_ is the same as EXTENDED_, but before call (for the duration of the call) of GetDefaultFunc()
	//    EAX, ECX (if not FASTCALL_ or THISCALL_), EDX (if not FASTCALL_), EBX, ESI, and EDI CPU registers
	//    are restored as they were at the moment of replaced function call
	//
	//  in most of the cases it is more convenient to use EXTENDED_
	//  but DIRECT_ works faster because of absence of the bridge to new_func
	//
	// calltype is a calling convention of the original replaced function:
	//  STDCALL_
	//  THISCALL_
	//  FASTCALL_
	//  CDECL_
	// it is necessary to specify right convention for EXTENDED_ hook to
	// create correct bridge to new_func
	//
	// CALL_, SPLICE_ hook is CODE_ patch
	// FUNCPTR_ hook is DATA_ patch
	//
	virtual HiHook* __stdcall WriteHiHook(_ptr_ address, int hooktype, int subtype, int calltype, void* new_func) = 0;

	///////////////////////////////////////////////////////////////////
	// Методы Create...
	// создают патч/хук так же как и соответствующие методы Write...,
	// но НЕ ПРИМЕНЯЮТ его
	// возвращают указатель на патч/хук
	// ENG
	// Create... methods
	// create patch/hook as corresponding Write... methods,
	// but DO NOT APPLY it
	// return a pointer to the patch/hook
	virtual Patch* __stdcall CreateBytePatch(_ptr_ address, int value) = 0;
	virtual Patch* __stdcall CreateWordPatch(_ptr_ address, int value) = 0;
	virtual Patch* __stdcall CreateDwordPatch(_ptr_ address, int value) = 0;
	virtual Patch* __stdcall CreateJmpPatch(_ptr_ address, _ptr_ to) = 0;
	virtual Patch* __stdcall CreateHexPatch(_ptr_ address, char* hex_str) = 0;
	virtual Patch* __stdcall CreateCodePatchVA(_ptr_ address, char* format, _dword_* va_args) = 0;
	virtual LoHook* __stdcall CreateLoHook(_ptr_ address, _LoHookFunc_ func) = 0;
	virtual HiHook* __stdcall CreateHiHook(_ptr_ address, int hooktype, int subtype, int calltype, void* new_func) = 0;

	////////////////////////////////////////////////////////////
	// Метод ApplyAll
	// применяет все патчи/хуки, созданные этим экземпляром PatcherInstance
	// всегда возвращает 1 (для совместимости с более ранними версиями библиотеки)
	// (см. Patch::Apply)
	// ENG
	// ApplyAll method
	// applies all the patches/hooks created with this PatcherInstance instance
	// always returns 1 (for the compatibility with the previous versions of the library)
	// (see Patch::Apply)
	virtual _bool_ __stdcall ApplyAll() = 0;

	////////////////////////////////////////////////////////////
	// Метод UndoAll
	// отменяет все патчи/хуки, созданные этим экземпляром PatcherInstance
	// т.е. для каждого из патчей/хуков вызывает метод Undo
	// всегда возвращает 1 (для совместимости с более ранними версиями библиотеки)
	// (см. Patch::Undo)
	// ENG
	// UndoAll method
	// cancels all the patches/hooks created with this PatcherInstance instance
	// i. e. calls Undo method for each patch/hook
	// always returns 1 (for the compatibility with the previous versions of the library)
	// (see Patch::Undo)
	virtual _bool_ __stdcall UndoAll() = 0;

	////////////////////////////////////////////////////////////
	// Метод DestroyAll
	// отменяет и безвозвратно уничтожает все патчи/хуки, созданные этим экземпляром PatcherInstance
	// т.е. для каждого из патчей/хуков вызывает метод Destroy
	// всегда возвращает 1 (для совместимости с более ранними версиями библиотеки)
	// (см. Patch::Destroy)
	// ENG
	// DestroyAll method
	// cancels and irretrievably destroys all the patches/hooks created with this PatcherInstance instance
	// i. e. calls Destroy method for each patch/hook
	// always returns 1 (for the compatibility with the previous versions of the library)
	// (see Patch::Destroy)
	virtual _bool_ __stdcall DestroyAll() = 0;

	// в оригинальном виде применение метода не предполагается,
	// смотрите (ниже) описание метода-оболочки WriteDataPatch
	// ENG
	// this method is not supposed to be used in the original form
	// see the description of the shell method WriteDataPatch (below)
	virtual Patch* __stdcall WriteDataPatchVA(_ptr_ address, char* format, _dword_* va_args);
	// в оригинальном виде применение метода не предполагается,
	// смотрите (ниже) описание метода-оболочки CreateDataPatch
	// ENG
	// this method is not supposed to be used in the original form
	// see the description of the shell method CreateDataPatch (below)
	virtual Patch* __stdcall CreateDataPatchVA(_ptr_ address, char* format, _dword_* va_args);


	// Метод GetLastPatchAt
	// возвращает NULL, если в окрестности адреса address не был применен ни один патч/хук,
	// созданный данным экземпляром PatcherInstance
	// иначе возвращает последний примененный патч/хук в окрестности адреса address,
	// созданный данным экземпляром PatcherInstance
	// ENG
	// GetLastPatchAt method
	// returns NULL if no patch/hook created with this PatcherInstance instance
	// was applied at (near) the specified address
	// otherwise returns the last patch/hook created with this PatcherInstance instance
	// and applied at (near) the specified address
	virtual Patch* __stdcall GetLastPatchAt(_ptr_ address) = 0;

	// Метод UndoAllAt
	// отменяет патчи примененные данным экземпляром PatcherInstance
	// в окрестности адреса address 
	// всегда возвращает 1 (для совместимости с более ранними версиями библиотеки)
	// (см. Patch::Undo)
	// ENG
	// UndoAllAt method
	// cancels all the patches/hooks created with this PatcherInstance instance
	// applied at (near) the specified address
	// always returns 1 (for the compatibility with the previous versions of the library)
	// (see Patch::Undo)
	virtual _bool_ __stdcall UndoAllAt(_ptr_ address) = 0;

	// Метод GetFirstPatchAt
	// возвращает NULL, если в окрестности адреса address не был применен ни один патч/хук,
	// созданный данным экземпляром PatcherInstance
	// иначе возвращает первый примененный патч/хук в окрестности адреса address,
	// созданный данным экземпляром PatcherInstance
	// ENG
	// GetFirstPatchAt method
	// returns NULL if no patch/hook created with this PatcherInstance instance
	// was applied at (near) the specified address
	// otherwise returns the first patch/hook created with this PatcherInstance instance
	// and applied at (near) the specified address
	virtual Patch* __stdcall GetFirstPatchAt(_ptr_ address) = 0;


	////////////////////////////////////////////////////////////
	// Метод Write
	// пишет по адресу address данные/код из памяти по адресу data размером size байт 
	// если is_code == 1, то создается и пишется CODE_ патч, если 0 - DATA_ патч.
	// Возвращает указатель на патч
	// Write method
	// writes data/code from memory at 'data' to memory at 'address' with the specified size
	// if is_code == 1 then CODE_ patch is created and applied, if is_code == 0 then DATA_ patch is created and applied.
	// Returns a pointer to the patch
	virtual Patch* __stdcall Write(_ptr_ address, _ptr_ data, _dword_ size, _bool_ is_code = 0) = 0;
	///////////////////////////////////////////////////////////////////
	// Метод CreatePatch
	// создаёт патч так же как и метод Write,
	// но НЕ ПРИМЕНЯЕТ его
	// возвращают указатель на патч
	// ENG
	// CreatePatch method
	// creates the same patch as Write method,
	// but DOES NOT APPLY it
	// returns a pointer to the patch
	virtual Patch* __stdcall CreatePatch(_ptr_ address, _ptr_ data, _dword_ size, _bool_ is_code = 0) = 0;


	//## ver 2.1
	// ENG
	//## ver 2.1

	////////////////////////////////////////////////////////////
	// Метод WriteLoHookEx
	// аналогичен методу WriteLoHook, но имеет дополнительный аргумент
	// stack_delta - размер памяти который можно поместить в стэк контекста
	// используя HookContext::esp и HookContext::Push внутри func.
	//
	// Возвращают указатель на LoHook хук
	// ENG
	// WriteLoHookEx method
	// this method is similar to WriteLoHook, but has an additional argument
	// stack_delta - the size of data which can be placed to the stack of the context
	// via HookContext::esp and HookContext::Push inside func.
	//
	// Returns a pointer to the LoHook hook
	virtual LoHook* __stdcall WriteLoHookEx(_ptr_ address, _LoHookFunc_ func, _dword_ stack_delta) = 0;
	// Метод CreateLoHookEx
	// создает хук так же как и WriteLoHookEx,
	// но НЕ ПРИМЕНЯЕТ его.
	// Возвращают указатель на LoHook хук
	// ENG
	// CreateLoHookEx method
	// creates the same hook as WriteLoHookEx method,
	// but DOES NOT APPLY it
	// returns a pointer to the LoHook hook
	virtual LoHook* __stdcall CreateLoHookEx(_ptr_ address, _LoHookFunc_ func, _dword_ stack_delta) = 0;


	// в оригинальном виде применение метода не предполагается,
	// смотрите (ниже) описание метода-оболочки WriteHexHook
	// ENG
	// this method is not supposed to be used in the original form
	// see the description of the shell method WriteHexHook (below)
	virtual LoHook* __stdcall WriteHexHookVA(_ptr_ address, _bool_ exec_default, char* hex_str, _dword_* va_args) = 0;
	// в оригинальном виде применение метода не предполагается,
	// смотрите (ниже) описание метода-оболочки CreateHexHook
	// ENG
	// this method is not supposed to be used in the original form
	// see the description of the shell method CreateHexHook (below)
	virtual LoHook* __stdcall CreateHexHookVA(_ptr_ address, _bool_ exec_default, char* hex_str, _dword_* va_args) = 0;



	// метод BlockAt устанавливает блок на определенный адрес (именно на конкретный адрес а не на окрестность)
	// для данного экземпляра PatcherInstance
	// после чего данный экземпляр PatcherInstance не может применять 
	// патчи по этому адресу
	// ENG
	// BlockAt method locks the specified address (exactly address, not area near it)
	// for this PatcherInstance instance
	// after this the PatcherInstance instance cannot apply
	// patches at this address
	virtual void __stdcall BlockAt(_ptr_ address) = 0;


	//## ver 2.6
	// ENG
	//## ver 2.6

	// метод BlockAllExceptVA устанавливает блок на все адреса кроме указанных в va_args
	// (так же как и метод BlockAt оперирует конкретными адресами а не окрестностями)
	// для данного экземпляра PatcherInstance
	// после чего данный экземпляр PatcherInstance не может применять 
	// патчи по всем адресам кроме указанных.
	// список адресов в va_args должен заканчиваться 0 (нулем)
	// ENG
	// BlockAllExceptVA method locks all the addresses except ones from va_args
	// (like BlockAt method it operates exact addresses and not areas near them)
	// for this PatcherInstance instance
	// after this the PatcherInstance instance cannot apply
	// patches at all addresses except listed ones.
	// the list of the addresses in va_args must be terminated with 0 (zero value)
	virtual void __stdcall BlockAllExceptVA(_dword_* va_args) = 0;



	//## ver 4.0
	// ENG
	//## ver 4.0

	// в оригинальном виде применение метода не предполагается,
	// смотрите (ниже) описание метода-оболочки WriteAsmPatch
	// ENG
	// this method is not supposed to be used in the original form
	// see the description of the shell method WriteAsmPatch (below)
	virtual Patch* __stdcall WriteAsmPatchVA(_ptr_ address, _dword_* va_args) = 0;

	// в оригинальном виде применение метода не предполагается,
	// смотрите (ниже) описание метода-оболочки CreateAsmPatch
	// ENG
	// this method is not supposed to be used in the original form
	// see the description of the shell method CreateAsmPatch (below)
	virtual Patch* __stdcall CreateAsmPatchVA(_ptr_ address, _dword_* va_args) = 0;

	// в оригинальном виде применение метода не предполагается,
	// смотрите (ниже) описание метода-оболочки WriteAsmHook
	// ENG
	// this method is not supposed to be used in the original form
	// see the description of the shell method WriteAsmHook (below)
	virtual LoHook* __stdcall WriteAsmHookVA(_ptr_ address, _dword_* va_args) = 0;

	// в оригинальном виде применение метода не предполагается,
	// смотрите (ниже) описание метода-оболочки CreateAsmHook
	// ENG
	// this method is not supposed to be used in the original form
	// see the description of the shell method CreateAsmHook (below)
	virtual LoHook* __stdcall CreateAsmHookVA(_ptr_ address, _dword_* va_args) = 0;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WriteAsmPatch пишет патч по адресу address
	// возвращает указатель на патч
	// аргументы ... - это строки, содержащие код на ассемблере
	// распознаются все инструкции, распознаваемые OllyDbg 1.04 (вплоть до MMX и amd 3DNow! включительно)
	// ВНИМАНИЕ! в отличие от OllyDbg, целые числа не имеющие префикса 0x или постфикса h читаются как ДЕСЯТЕРИЧНЫЕ!
	// поэтому не забывайте писать шеснадцатеричные числа явно
	// В одной строке может быть несколько инструкций, разделенных символом ';'
	// Код на ассемблере может содержать метки;
	// объявление метки - label_name: (имя метки, двоеточие),
	// использование label_name (имя без двоеточия);
	// максимальная длина имени метки - 39 символов, имя может содержать буквы, цифры, символы '_' и '.';
	// код может содержать псевдоинструкцию times (пишет заданное количество раз следующую инструкцию)
	// например, результатом "times 3 nop" будет код 90 90 90
	// код может содержать псевдоинструкцию _ExecCopy <адрес>, <размер> (пишет код из памяти по адресу <адрес> размером <размер>)
	// код может содержать псевдоинструкции db <число>, dw <число>, dd <число или метка>.
	// Строка может содержать формат-символы %d. В этом случае за строкой должно следовать соответствующее количество четырехбайтовых чисел (знаковые/беззнаковые/адреса/...)
	// ВНИМАНИЕ! последним аргументом (строкой) должен обязательно быть 0 (NULL)!
	// абстрактный пример: WriteAsmPatch(0x112233, "begin: call %d", MyFunc, "jmp begin", "jne dword [%d]", 0xAABBCC, 0);
	// ENG
	// WriteAsmPatch writes a patch at the specified address
	// returns a pointer to the patch
	// ... arguments are strings containing assembler code
	// all instructions supported by OllyDbg 1.04 (up to MMX and amd 3DNow! inclusively) are correctly recognized by this method
	// ATTENTION! unlike OllyDbg, integers without 0x prefix or h postfix are counted as DECIMAL!
	// so, do not forget to write hexadecimal numbers explicitly
	// One string may contain several instructions, separated with ';' symbol
	// Assembler code may contain labels;
	// label declaration - label_name: (label name, colon),
	// label use - label_name (name without colon);
	// maximum label name length is 39 symbols, name may contain letters, digits, '_' and '.' symbols
	// the code may contain pseudo-instruction times (writes the next instruction the specified number of times)
	// for example, the string "times 3 nop" will result in code 90 90 90
	// the code may contain pseudo-instruction _ExecCopy <address>, <size> (writes the code from memory at the specified address with the specified size)
	// the code may contain pseudo-instructions db <number>, dw <number>, dd <number or label>.
	// The string may contain format-symbols %d. In this case the corresponding amount of 4-byte numbers (signed/unsigned/addresses/...) must follow the string
	// ATTENTION! the last argument (sting) must be 0 (NULL)!
	// an abstract example: WriteAsmPatch(0x112233, "begin: call %d", MyFunc, "jmp begin", "jne dword [%d]", 0xAABBCC, 0);
	inline Patch* WriteAsmPatch(_ptr_ address, ...)
	{
		return WriteAsmPatchVA(address, (_dword_*)((_ptr_)&address + 4));
	}
	////////////////////////////////////////////////////////////
	// Метод CreateAsmPatch
	// создает патч так же как и метод WriteAsmPatch,
	// но не применяет его
	// возвращает указатель на патч
	// ENG
	// CreateAsmPatch method
	// creates the same patch as WriteAsmPatch method,
	// but DOES NOT APPLY it
	// returns a pointer to the patch
	inline Patch* CreateAsmPatch(_ptr_ address, ...)
	{
		return CreateAsmPatchVA(address, (_dword_*)((_ptr_)&address + 4));
	}

	////////////////////////////////////////////////////////////
	// WriteAsmHook пишет по адресу address примитивный хук
	// а именно LoHook без вызова высокоуровневой функции.
	// тело хука пишется прямо в вызове CreateAsmHook или WriteAsmHook
	// таким же образом как пишется патч с помощью WriteAsmPatch (см. WriteAsmPatch)
	// В отличие метода WriteAsmPatch здесь код может содержать (а может не содержать) псевдокоманду _ExecDefault,
	// которая выполняет затертый хуком код
	// ВНИМАНИЕ! в коде не может быть больше одной псевдокоманды _ExecDefault
	// возвращает указатель на LoHook хук
	// ВНИМАНИЕ! последним аргументом (строкой) должен обязательно быть 0 (NULL)!
	// абстрактный пример: WriteAsmHook(0x112233, "cmp eax, 0; jne SkipDefault; _ExecDefault; jmp End; SkipDefault: mov ecx, 2; End: retn", 0);
	// ENG
	// WriteAsmHook writes a primitive hook at the specified address
	// i. e. LoHook without high-level function call.
	// hook body is passed directly to CreateAsmHook or WriteAsmHook call as argument/arguments
	// similarly to WriteAsmPatch method (see WriteAsmPatch)
	// Unlike WriteAsmPatch method in this method the code may contain (or may not contain) pseudo-instruction _ExecDefault,
	// which executes the original code replaced by hook
	// ATTENTION! code cannot contain _ExecDefault pseudo-instruction more than once
	// returns a pointer to the LoHook hook
	// ATTENTION! the last argument (sting) must be 0 (NULL)!
	// an abstract example: WriteAsmHook(0x112233, "cmp eax, 0; jne SkipDefault; _ExecDefault; jmp End; SkipDefault: mov ecx, 2; End: retn", 0);
	inline LoHook* WriteAsmHook(_ptr_ address, ...)
	{
		return WriteAsmHookVA(address, (_dword_*)((_ptr_)&address + 4));
	}

	////////////////////////////////////////////////////////////
	// Метод CreateAsmHook
	// создает хук так же как и WriteAsmHook,
	// но НЕ ПРИМЕНЯЕТ его.
	// Возвращает указатель на LoHook хук
	// ENG
	// CreateAsmHook method
	// creates the same hook as WriteAsmHook method,
	// but DOES NOT APPLY it
	// returns a pointer to the LoHook hook
	inline LoHook* CreateAsmHook(_ptr_ address, ...)
	{
		return CreateAsmHookVA(address, (_dword_*)((_ptr_)&address + 4));
	}



	// WriteHexHook пишет по адресу address самый примитивный хук
	// а именно LoHook без вызова высокоуровневой функции.
	// тело хука пишется прямо в вызове CreateHexHook или WriteHexHook
	// таким же образом как пишется патч с помощью WriteCodePatch (см. WriteCodePatch)
	//
	// exec_default - выполнять ли затертый хуком код после выполнения тела хука
	// возвращает указатель на LoHook хук
	// ENG
	// WriteHexHook writes the most primitive hook at the specified address
	// i. e. LoHook without high-level function call.
	// hook body is passed directly to CreateHexHook or WriteHexHook call as argument/arguments
	// similarly to WriteCodePatch method (see WriteCodePatch)
	//
	// exec_default determines if the original code replaced by hook will be executed after the hook body
	// returns a pointer to the LoHook hook
	inline LoHook* WriteHexHook(_ptr_ address, _bool_ exec_default, char* format, ...)
	{
		return WriteHexHookVA(address, exec_default, format, (_dword_*)((_ptr_)&format + 4));
	}
	// Метод CreateHexHook
	// создает хук так же как и WriteHexHook,
	// но НЕ ПРИМЕНЯЕТ его.
	// Возвращает указатель на LoHook хук
	// ENG
	// CreateHexHook method
	// creates the same hook as WriteHexHook method,
	// but DOES NOT APPLY it
	// returns a pointer to the LoHook hook
	inline LoHook* CreateHexHook(_ptr_ address, _bool_ exec_default, char* format, ...)
	{
		return CreateHexHookVA(address, exec_default, format, (_dword_*)((_ptr_)&format + 4));
	}



	////////////////////////////////////////////////////////////
	// Метод WriteCodePatch
	// пишет по адресу address позледовательность байт,
	// определенную format и ...
	// (создает и применяет CODE_ патч)
	// Возвращает указатель на патч
	// format - си-строка может содержать шестнадцатеричные цифры
	// 0123456789ABCDEF (только верхний регистр!),
	// а так же специальные формат-символы (нижний регистр!):
	// %b - (byte) пишет однобайтовое число из ...
	// %w - (word) пишет двухбайтовое число из ...
	// %d - (dword) пишет четырехбайтовое число из ...
	// %j - пишет jmp на адрес из ...
	// %с - пишет сall ...
	// %m - копирует код по адресу ... размером ... (т.е. читает 2 аргумента из ...)
	//      копирование происходит посредством MemCopyCodeEx (см. описание)
	// %% - пишет строку с формат-символами из ... 
	// %o - (offset) помещает по адресу из аргумента смещение позиции в
	//      Complex коде, относительно начала Complex кода.
	//      (этот символ ничего не добавляет в сам Complex код)
	// %n - пишет nop опкоды, количеством равным ...                                  \
    #0: - #9: - устанавливает метку (от 0 до 9) к которой можно перейти с помощью #0 - #9                              \
    #0 -  #9  - пишет отностельный адрес после опкодов EB, 70 - 7F, E8, E9, 0F80 - 0F8F
	//      соответствующей метки; после других опкодов ничего не пишет
	// ~b - берет из ... абсолютный адрес и пишет относительное смещение до него
	//      размером в 1 байт (используется для опкодов EB, 70 - 7F)
	// ~d - берет из ... абсолютный адрес и пишет относительное смещение до него
	//      размером в 4 байта (используется для опкодов E8, E9, 0F 80 - 0F 8F)
	// %. - ничего не делает ( как и любой другой не объявленный выше символ после % ) 
	// абстрактный пример:
	// Patch* p = pi->WriteCodePatch(address,
	//  "#0: %%",
	//  "B9 %d %%", this,     // mov ecx, this  // 
	//  "BA %d %%", this->context,   // mov edx, context  // 
	//  "%c %%", func,      // call func  // 
	//  "83 F8 01 %%",      // cmp eax, 1
	//  "0F 85 #7 %%",       // jne long to label 7 (if func returns 0)
	//  "83 F8 02 %%",      // cmp eax, 2
	//  "0F 85 ~d %%", 0x445544,   // jne long to 0x445544 (if func returns 0)
	//  "EB #0 %%",       // jmp short to label 0
	//  "%m %%", address2, size,   // exec  code copy from address2
	//  "#7: FF 25 %d %.", &return_address); // jmp [&return_address]
	// ENG
	// WriteCodePatch method
	// writes a sequence of bytes specified by format and ...
	// at the specified address
	// (creates and applies CODE_ patch)
	// Returns a pointer to the patch
	// format is C-style string that may contain hexadecimal digits
	// 0123456789ABCDEF (only uppercase!),
	// and also specific format-symbols (lowercase!):
	// %b - (byte) writes 1-byte number form ...
	// %w - (word) writes 2-byte number form ...
	// %d - (dword) writes 4-byte number form ...
	// %j - writes jmp to address from ...
	// %с - writes сall ...
	// %m - copies code from address ... with the size ... (i. e. reads 2 arguments from ...)
	//      copying process uses MemCopyCodeEx (see the description)
	// %% - writes the string with the format symbols from ...
	// %o - (offset) writes a shift of the current position in Complex code, relative to
	//      the start of the Complex code, to the address specified in an argument.
	//      (nothing is added to Complex code by this symbol)
	// %n - writes nop opcodes in ... amount                                 \
    #0: - #9: - set the label (from 0 to 9) which can be accessed using #0 - #9                              \
    #0 -  #9  - writes the relative address of the corresponding label after 
	//      opcodes EB, 70 - 7F, E8, E9, 0F80 - 0F8F; writes nothing after another opcodes
	// ~b - takes an absolute address from ... and writes the relative shift to it
	//      with the size of 1 byte (used for opcodes EB, 70 - 7F)
	// ~d - takes an absolute address from ... and writes the relative shift to it
	//      with the size of 4 bytes (used for opcodes E8, E9, 0F 80 - 0F 8F)
	// %. - does nothing ( as any symbol after % except ones above ) 
	// abstract example:
	// Patch* p = pi->WriteCodePatch(address,
	//  "#0: %%",
	//  "B9 %d %%", this,     // mov ecx, this  // 
	//  "BA %d %%", this->context,   // mov edx, context  // 
	//  "%c %%", func,      // call func  // 
	//  "83 F8 01 %%",      // cmp eax, 1
	//  "0F 85 #7 %%",       // jne long to label 7 (if func returns 0)
	//  "83 F8 02 %%",      // cmp eax, 2
	//  "0F 85 ~d %%", 0x445544,   // jne long to 0x445544 (if func returns 0)
	//  "EB #0 %%",       // jmp short to label 0
	//  "%m %%", address2, size,   // exec  code copy from address2
	//  "#7: FF 25 %d %.", &return_address); // jmp [&return_address]
	inline Patch* WriteCodePatch(_ptr_ address, char* format, ...)
	{
		return WriteCodePatchVA(address, format, (_dword_*)((_ptr_)&format + 4));
	}

	////////////////////////////////////////////////////////////
	// Метод CreateCodePatch
	// создает патч так же как и метод WriteCodePatch,
	// но не применяет его
	// возвращает указатель на патч
	// ENG
	// CreateCodePatch method
	// creates the same patch as WriteCodePatch method,
	// but does not apply it
	// returns a pointer to the patch
	inline Patch* CreateCodePatch(_ptr_ address, char* format, ...)
	{
		return CreateCodePatchVA(address, format, (_dword_*)((_ptr_)&format + 4));
	}


	////////////////////////////////////////////////////////////
	// Метод WriteDataPatch
	// пишет по адресу address последовательность байт,
	// определенную format и ...
	// (создает и применяет DATA_ патч)
	// Возвращает указатель на патч
	// format - си-строка может содержать шестнадцатеричные цифры
	// 0123456789ABCDEF (только верхний регистр!),
	// а так же специальные формат-символы (нижний регистр!):
	// %b - (byte) пишет однобайтовое число из ...
	// %w - (word) пишет двухбайтовое число из ...
	// %d - (dword) пишет четырехбайтовое число из ...
	// %m - копирует данные по адресу ... размером ... (т.е. читает 2 аргумента из ...)
	// %% - пишет строку с формат-символами из ... 
	// %o - (offset) помещает по адресу из аргумента смещение позиции в
	//      Complex данных, относительно начала Complex данных.
	//      (этот символ ничего не добавляет в сами Complex данные)
	// %. - ничего не делает ( как и любой другой не объявленный выше символ после % ) 
	// абстрактный пример:
	// Patch* p = pi->WriteDataPatch(address,
	//  "FF FF FF %d %%", var, 
	//  "%m %%", address2, size, 
	//  "AE %.");
	// ENG
	// WriteDataPatch method
	// writes a sequence of bytes specified by format and ...
	// at the specified address
	// (creates and applies DATA_ patch)
	// Returns a pointer to the patch
	// format is C-style string that may contain hexadecimal digits
	// 0123456789ABCDEF (only uppercase!),
	// and also specific format-symbols (lowercase!):
	// %b - (byte) writes 1-byte number form ...
	// %w - (word) writes 2-byte number form ...
	// %d - (dword) writes 4-byte number form ...
	// %m - copies data from address ... with the size ... (i. e. reads 2 arguments from ...)
	// %% - writes the string with the format symbols from ...
	// %o - (offset) writes a shift of the current position in Complex data, relative to
	//      the start of the Complex data, to the address specified in an argument.
	//      (nothing is added to Complex data by this symbol)
	// %. - does nothing ( as any symbol after % except ones above ) 
	// abstract example:
	// Patch* p = pi->WriteDataPatch(address,
	//  "FF FF FF %d %%", var, 
	//  "%m %%", address2, size, 
	//  "AE %.");
	inline Patch* WriteDataPatch(_ptr_ address, char* format, ...)
	{
		return WriteDataPatchVA(address, format, (_dword_*)((_ptr_)&format + 4));
	}

	////////////////////////////////////////////////////////////
	// Метод CreateDataPatch
	// создает патч так же как и метод WriteDataPatch,
	// но не применяет его
	// возвращает указатель на патч
	// ENG
	// CreateDataPatch method
	// creates the same patch as WriteDataPatch method,
	// but does not apply it
	// returns a pointer to the patch
	inline Patch* CreateDataPatch(_ptr_ address, char* format, ...)
	{
		return CreateDataPatchVA(address, format, (_dword_*)((_ptr_)&format + 4));
	}

	inline Patch* __stdcall WriteDword(_ptr_ address, const char* value)
	{
		return WriteDword(address, (int)value);
	}

};

// Класс Patcher
// ENG
// Patcher class
class Patcher
{
public:
	// основные методы:
	// ENG
	// main methods:

	///////////////////////////////////////////////////
	// Метод CreateInstance
	// создает экземпляр класса PatcherInstance, который 
	// непосредственно позволяет создавать патчи и хуки и
	// возвращает указатель на этот экземпляр.
	// owner - уникальное имя экземпляра PatcherInstance
	// метод возвращает NULL, если экземпляр с именем owner уже создан
	// если owner == NULL или owner == "" то 
	// экземпляр PatcherInstance будет создан с именем модуля из
	// которого была вызвана функция.
	// ENG
	// CreateInstance method
	// creates PatcherInstance instance, which 
	// directly allows to create patches and hooks,
	// returns a pointer to this instance.
	// owner is an unique name of PatcherInstance instance
	// returns NULL if an instance with this name was already created
	// if owner == NULL or owner == "" then
	// PatcherInstance instance is created with the name - name of the module
	// from which this method is called.
	virtual PatcherInstance* __stdcall CreateInstance(char* owner) = 0;

	///////////////////////////////////////////////////
	// Метод GetInstance
	// Возвращает указатель на экземпляр PatcherInstance
	// с именем owner.
	// метод возвращает NULL в случае, если 
	// экземпляр с именем owner не существует (не был создан)
	// в качестве аргумента можно передавать имя модуля.
	// Используется для :
	// - проверки активен ли некоторый мод, использующий patcher_x86.dll
	// - получения доступа ко всем патчам и хукам некоторого мода,
	//   использующего patcher_x86.dll
	// ENG
	// GetInstance method
	// Returns a pointer to the PatcherInstance instance
	// with the name owner.
	// this method returns NULL if
	// an instance with the name owner is not exists (was not created)
	// the name of the module can be passed as an argument.
	// Used for :
	// - check if some mod that uses patcher_x86.dll is active
	// - get the access to all the patches and hooks of some mod
	//   that uses patcher_x86.dll
	virtual PatcherInstance* __stdcall GetInstance(char* owner) = 0;

	///////////////////////////////////////////////////
	// Метод GetLastPatchAt
	// возвращает NULL, если в окрестности адреса address не был применен ни один патч/хук
	// иначе возвращает последний примененный патч/хук в окрестности адреса address
	// можно последовательно пройтись по всем патчам в заданной окрестности 
	// используя этот метод и Patch::GetAppliedBefore
	// ENG
	// GetLastPatchAt method
	// returns NULL if no patch/hook was applied at (near) the specified address
	// otherwise returns the last patch/hook applied at (near) the specified address
	// it is possible to go through all the patches at (near) the specific address
	// using this method and Patch::GetAppliedBefore
	virtual Patch* __stdcall GetLastPatchAt(_ptr_ address);

	///////////////////////////////////////////////////
	// Метод UndoAllAt
	// отменяет все патчи/хуки в окрестности адреса address
	// всегда возвращает 1 (для совместимости с более ранними версиями библиотеки)
	// ENG
	// UndoAllAt method
	// cancels all the patches/hooks applied at (near) the specified address
	// always returns 1 (for the compatibility with the previous versions of the library)
	virtual Patch* __stdcall UndoAllAt(_ptr_ address);

	///////////////////////////////////////////////////
	// Метод SaveDump
	// сохраняет в файл с именем file_name:
	// - количество и имена всех экземпляров PatcherInstance
	// - количество всех примененных патчей/хуков
	// - список всех примененных патчей и хуков с их адресами установки, размерами, глобальной очередностью применения, собственниками (именами PatcherInstance)
	// ENG
	// SaveDump method
	// save the information to file with name file_name:
	// - the amount and the names of each of PatcherInstance instances
	// - the amount of all the patches/hooks applied
	// - the list of all the patches/hooks applied with their addresses, sizes, global apply order, owners (PatcherInstance names)
	virtual void __stdcall SaveDump(char* file_name) = 0;

	///////////////////////////////////////////////////
	// Метод SaveLog
	// сохраняет в файл с именем file_name лог 
	// если логирование выключено в логе будет 0 записей.
	// включить логирование можно создав в директории библиотеки
	// текстовый файл patcher_x86.ini c содержимым: Logging = 1
	// ENG
	// SaveLog method
	// save the log to file with name file_name:
	// if logging is disable the log will contain 0 records.
	// you can enable logging by creating patcher_x86.ini in the folder of library 
	// with the record: Logging = 1
	virtual void __stdcall SaveLog(char* file_name) = 0;

	///////////////////////////////////////////////////
	// Метод GetMaxPatchSize
	// Библиотека patcher_x86.dll накладывает некоторые ограничения
	// на максимальный размер патча,
	// какой - можно узнать с помощью метода GetMaxPatchSize
	// (на данный момент это 262144 байт, т.е. дохрена :) )
	// ENG
	// GetMaxPatchSize method
	// The library patcher_x86.dll has some limits
	// on the maximum patch size,
	// you can get to know this limit with GetMaxPatchSize method call
	// (currently it is 262144 bytes, i. e. a lot :) )
	virtual int __stdcall GetMaxPatchSize() = 0;

	// дополнительные методы:
	// ENG
	// additional methods:

	///////////////////////////////////////////////////
	// Метод WriteComplexDataVA
	// в оригинальном виде применение метода не предполагается,
	// смотрите (ниже) описание метода-оболочки WriteComplexString
	// ENG
	// WriteComplexDataVA method
	// this method is not supposed to be used in the original form
	// see the description of the shell method WriteComplexString (below)
	virtual int __stdcall WriteComplexDataVA(_ptr_ address, char* format, _dword_* args) = 0;

	///////////////////////////////////////////////////
	// метод GetOpcodeLength
	// т.н. дизассемблер длин опкодов
	// возвращает длину в байтах опкода по адресу p_opcode
	// возвращает 0, если опкод неизвестен
	// ENG
	// GetOpcodeLength method
	// so-called opcode length disassembler
	// returns the length in bytes of an opcode from p_opcode address
	// returns 0 if an opcode is unknown
	virtual int __stdcall GetOpcodeLength(_ptr_ p_opcode) = 0;

	///////////////////////////////////////////////////
	// метод MemCopyCode
	// копирует код из памяти по адресу src в память по адресу dst
	// MemCopyCode копирует всегда целое количество опкодов размером >= size. Будьте внимательны!
	// возвращает размер скопированного кода.
	// отличается действием от простого копирования памяти тем,
	// что корректно копирует опкоды E8 (call), E9 (jmp long), 0F80 - 0F8F (j** long)
	// c относительной адресацией не сбивая в них адреса, если инструкции 
	// направляют за пределы копируемого блокая.
	// 
	// ENG
	// MemCopyCode method
	// copies the code from memory at src address to memory at dst address
	// MemCopyCode always copies an integer amount of opcodes with summary size >= size from argument. Be careful!
	// returns the size of the copied code.
	// it is different from direct memory copy, because
	// it correctly copies opcodes E8 (call), E9 (jmp long), 0F80 - 0F8F (j** long)
	// with relative addressing without changing the absolute address of jump/call, if this address
	// is out of copied block
	// 
	virtual int __stdcall MemCopyCode(_ptr_ dst, _ptr_ src, _dword_ size) = 0;

	///////////////////////////////////////////////////
	// Метод GetFirstPatchAt
	// возвращает NULL, если в окрестности адреса address не был применен ни один патч/хук
	// иначе возвращает первый примененный патч/хук в окрестности адреса address
	// можно последовательно пройтись по всем патчам в заданной окрестности 
	// используя этот метод и Patch::GetAppliedAfter
	// ENG
	// GetFirstPatchAt method
	// returns NULL if no patch/hook was applied at (near) the specified address
	// otherwise returns the first patch/hook applied at (near) the specified address
	// it is possible to go through all the patches at (near) the specific address
	// using this method and Patch::GetAppliedAfter
	virtual Patch* __stdcall GetFirstPatchAt(_ptr_ address);

	///////////////////////////////////////////////////
	// метод MemCopyCodeEx
	// копирует код из памяти по адресу src в память по адресу dst
	// возвращает размер скопированного кода.
	// отличается от MemCopyCode тем,
	// что корректно копирует опкоды EB (jmp short), 70 - 7F (j** short)
	// c относительной адресацией не сбивая в них адреса, если инструкции 
	// направляют за пределы копируемого блока (в этом случае они заменяются на
	// соответствующие E9 (jmp long), 0F80 - 0F8F (j** long) опкоды.
	// Внимание! Из-за этого размер скопированного кода может оказаться значительно 
	// больше копируемого.
	// ENG
	// MemCopyCodeEx method
	// copies the code from memory at src address to memory at dst address
	// returns the size of the result code.
	// it is different from MemCopyCode, because
	// it correctly copies opcodes EB (jmp short), 70 - 7F (j** short)
	// with relative addressing without changing the absolute address of jump/call, if this address
	// is out of the copied block (in this case they are replaced with
	// corresponding E9 (jmp long), 0F80 - 0F8F (j** long) opcodes.
	// Attention! This may cause the significant growth of the result code
	// comparing to code from src.
	virtual int __stdcall MemCopyCodeEx(_ptr_ dst, _ptr_ src, _dword_ size) = 0;


	// ver 2.3
	// ENG
	// ver 2.3

	// Метод VarInit
	// инициализирует "переменную" c именем name и устанавливает значение "переменной" равным value
	// если "переменная" с таким именем уже существует, то просто устанавливает ее значение равным value
	// возвращает указатель на "переменную" в случае успеха и NULL в противном случае
	// ENG
	// VarInit method
	// initializes a "variable" with the specified name and set its value
	// if the "variable" with this name already exists then its value is just setted
	// returns the pointer to the "variable" in case of success or NULL otherwise
	virtual Variable* __stdcall VarInit(char* name, _dword_ value) = 0;
	// Метод VarFind
	// возвращает указатель на "переменную" с именем name, если такая была инициализирована
	// если нет, возвращает NULL
	// ENG
	// VarFind method
	// returns a pointer to the "variable" with the specified name, if it was initialized
	// otherwise returns NULL
	virtual Variable* __stdcall VarFind(char* name) = 0;


	// ver 2.6
	// ENG
	// ver 2.6

	// Метод PreCreateInstance
	// Создает неполноценный экземпляр PatcherInstance с указанным именем.
	// PatcherInstance созданный таким образом не может создавать патчи.
	// Этот неполноценный экземпляр используется для применения методов PatcherInstance::BlockAt и PatcherInstance::BlockAllExceptVA
	// чтобы можно было заблокировать адреса до того как данный PatcherInstance будет полноценно создан с помощью CreateInstance
	// ENG
	// PreCreateInstance method
	// Creates an imperfect PatcherInstance instance with the specified name.
	// PatcherInstance created this way cannot create patches.
	// This imperfect instance is used for PatcherInstance::BlockAt and PatcherInstance::BlockAllExceptVA methods applying
	// in order to make it possible to block addresses before this PatcherInstance will be perfectly created via CreateInstance
	virtual PatcherInstance* __stdcall PreCreateInstance(char* name) = 0;


	// ver 4.1
	// ENG
	// ver 4.1

	// описание в разработке...
	// ENG
	// the description is under construction...
	virtual int __stdcall WriteAsmCodeVA(_ptr_ address, _dword_* args) = 0;
	virtual _ptr_ __stdcall CreateCodeBlockVA(_dword_* args) = 0;


	// метод VarGetValue возвращает значение "переменной" c именем name
	// если "переменная" с таким именем не была инициализирована, возвращает default_value
	// ENG
	// VarGetValue method returns the value of the "variable" with the specified name
	// if the "variable" with this name was not initialized, default_value is returned
	template<typename ValueType>
	inline ValueType VarGetValue(char* name, ValueType default_value)
	{
		if (sizeof(ValueType) > 4) return default_value;
		Variable* v = VarFind(name);
		if (v == NULL) return default_value;
		return (ValueType)v->GetValue();
	}

	// метод VarValue возвращает ссылку на значение "переменной" c именем name
	// если "переменная" с таким именем не была инициализирована, инициализирует ее и устанавливает значение равным 0
	// внимание, обращение к значению переменной по ссылке непотокобезопасно
	// ENG
	// VarValue method returns a reference to the "variable" value with the specified name
	// if the "variable" with this name was not initialized, initializes it and set its value to 0
	// attention, the access to the variable via this reference is not thread-safe
	template<typename ValueType>
	inline ValueType& VarValue(char* name)
	{
		if (sizeof(ValueType) > 4) __asm {__asm int 3};

		Variable* v = VarFind(name);
		if (v == NULL) v = VarInit(name, 0);

		if (v == NULL) __asm {__asm int 3};

		return (ValueType&)*v->GetPValue();
	}



	////////////////////////////////////////////////////////////////////
	// метод WriteComplexData
	// является более удобным интерфейсом  
	// метода WriteComplexDataVA
	// этот метод определен здесь а не в библиотеке, т.к. его вид 
	// отличается в Си и Делфи
	// Функционал метода почти тот же что и у PatcherInstance::WriteCodePatch
	// (см. описание этого метода)
	// то есть метод пишет по адресу address, последовательность байт,
	// определенную аргументами format и ...,
	// НО! НЕ создает экземпляр класса Patch, со всеми вытекающими (т.е. не позволяя отменить правку, получить доступ к правке из другого мода и т.д.)
	// ВНИМАНИЕ!
	// Используйте этот метод только для динамического создания блоков
	// кода, т.е. пишите этим методом только в свою память, 
	// а в код модифицируемой программы только с помощью
	// PatcherInstance::WriteCodePatch
	//
	// ENG
	// WriteComplexData method
	// it is the more convenient interface
	// of WriteComplexDataVA method
	// this method is defined here and not in the library because its form
	// is different for C and Delphi
	// The method functionality is almost the same as PatcherInstance::WriteCodePatch functionality
	// (see its description)
	// i. e. this method writes a sequence of bytes specified by format and ...
	// at the specified address
	// BUT! It DOES NOT create an instance of Patch class, with all the consequences (i. e. it is not possible to cancel this fix, to get access to this fix from another mod, etc.)
	// ATTENTION!
	// Use this method only for dynamic code blocks creation
	// i. e. write with this method only at memory allocated by you
	// and write at the modified program code only via
	// PatcherInstance::WriteCodePatch
	//
	inline _ptr_ WriteComplexData(_ptr_ address, char* format, ...)
	{
		return WriteComplexDataVA(address, format, (_dword_*)((_ptr_)&format + 4));
	}



	inline HiHook* GetFirstHiHookAt(_ptr_ address)
	{
		Patch* p = GetFirstPatchAt(address);
		while (true)
		{
			if (p == 0) return 0;
			if (p->GetType() == HIHOOK_) return (HiHook*)p;
			p = p->GetAppliedAfter();
		}
	}

	inline HiHook* GetLastHiHookAt(_ptr_ address)
	{
		Patch* p = GetLastPatchAt(address);
		while (true)
		{
			if (p == 0) return 0;
			if (p->GetType() == HIHOOK_) return (HiHook*)p;
			p = p->GetAppliedBefore();
		}
	}

};





// восстанавливаем выравнивание членов структур и классов
// ENG
// restore class and structure members alignment
#pragma pack(pop)

//////////////////////////////////////////////////////////////////

// функция GetPatcher
// загружает библиотеку и, с помощью вызова единственной экспортируемой 
// функции _GetPatcherX86@0, возвращает указатель на объект Patcher,
// посредством которого доступен весь функционал библиотеки patcher_x86.dll,
// возвращает NULL при неудаче
// функцию вызывать 1 раз, что очевидно из ее определения
// ENG
// GetPatcher function
// loads the library and via the only exported function _GetPatcherX86@0 call
// returns the pointer to the Patcher object,
// which provides access to the whole patcher_x86.dll library functionality,
// returns NULL if fails
// function must be called once, which is obvious from its definition
#include <windows.h>
inline Patcher* GetPatcher()
{
	static int calls_count = 0;
	if (calls_count > 0) return NULL;
	calls_count++;
	HMODULE pl = LoadLibraryA("patcher_x86.dll");
	if (pl)
	{
		FARPROC f = GetProcAddress(pl, "_GetPatcherX86@0");
		if (f) return CALL_0(Patcher*, __stdcall, f);
	}
	return NULL;
}

