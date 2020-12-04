////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ���������� patcher_x86.dll 
// ���������������� ��������(���������)
// ��������� �����: ������� ��������� (baratorch), e-mail: baratorch@yandex.ru
// ����� ���������� �������������� ����� (LoHook) ������� �������������� � Berserker (�� ERA)
// ENG
// library patcher_x86.dll 
// distributed free of charge
// author: Barinov Alexander (baratorch), e-mail: baratorch@yandex.ru
// the design of low-level hooks (LoHook) partially got from Berserker's ERA
////////////////////////////////////////////////////////////////////////////////////////////////////////////

// ������ 4.2
// ENG
// version 4.2

////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ��������.
//
// ! ���������� �������������:
//  - ������� ��������������� ���������������� 
//    ����������� ��� ��������� ������ � �����
//    � ��� ������� ���������.
//  - �������������� �����������: ������������ ���� ������� � �������
//    ���������� ��� � ���������� ��������� ������� jmp � call c 
//    ������������� ����������
// ! ���������� ���������
//  - ������������� ��� ������� ��� � ������� �����.
//  - ������������� ��������������� ����, ������� ������������ ������� �
//    � ������� ���� �� ����, �� �������� � ��������� ����������,
//    �����, � �������� � ������������ ���.
//  - ������������� ��������������� ���� ���� �� ������
//    �� �������� � �������� ��� ���� ���������������� �����
//    ������������� ������ ����������
//  - ������������� �������������� ���� � ��������������� �������� �
//    ��������� ����������, �����, ��������� ���� � ������ �������� � ���
//  - �������� ����� ���� � ���, ������������� � ������� ���� ����������.
//  - ������ ������������ �� ������������ ���, ������������ ����������
//  - ������ ����� ��� (������������ ����������) ��������� ������������ ����/���
//  - �������� ������ ������ �� ���� ������/�����, ������������� �� ������ ����� 
//    � ������� ���� ����������
//  - ����� � ������ ���������� ������������� ����� �� ������ �����
//    (������������ ��� ����������) 1) ������� � ���� ����� ��������� ���:
//        - ��������������� �����/���� ������� ������� �� ���� �����
//        - ��������������� �����/���� ������������� ���� ������� �� ���������
//        - ��������������� ����� ������ ����� � ��������
//    � ��� �� 2) ����� ����������� ���������� ���� (����� �������) ���� ������ 
//    � ����� ������������� � ������� ���� ���������� � ���������� ������ �������.
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
// �����������.
//
// �� ��������� � patcher_x86.dll ����������� ���������, ����� �������� ���,
// ���������� � ��� �� ����� ������� ���� patcher_x86.ini c ������������
// �������: Logging = 1 (Logging = 0 - ��������� ����������� �����)
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
// ������� �������������.
//
// 1) ������ ��� ������ 1 ��� ������� ������� GetPatcher(), �������� ���������
//  ��������: Patcher* _P = GetPatcher();
// 2) ����� � ������� ������ Pather::CreateInstance ����� �������  
// ��������� Pat�herInstance �� ����� ���������� ������
//  ��������: Pat�herInstance* _PI = _P->CreateInstance("MyMod");
// 3)  ����� ������������ ������ ������� Pat�her � Pat�herInstance
// ��������������� ��� ������ � ������� � ������
// ENG
// HOW TO USE.
//
// 1) each mod must call the function GetPatcher() once and save its result
//  example: Patcher* _P = GetPatcher();
// 2) then you need to create instance of Pat�herInstance with unique name
// with Pather::CreateInstance method
//  example: Pat�herInstance* _PI = _P->CreateInstance("MyMod");
// 3)  then use Pat�her and Pat�herInstance methods
// directly to work with patches and hooks
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#define _byte_ unsigned __int8
#define _word_ unsigned __int16
#define _dword_ unsigned __int32


//������� CALL_? ��������� �������� ������������ ������� �� ������������� ������
//������������ � ��� ����� ��� ������ �������
//���������� � ������� HiHook::GetDefaultFunc � HiHook::GetOriginalFunc
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


// _bool_ - 4-� �������� ���������� ���, ��� BOOL � Win32 API
// ���� ����� ���������, ����� �������� �� BOOL ��� ������������ bool ��������
// ENG
// _bool_ - 4-byte boolean type, as BOOL in Win32 API
// if you want this much you may replace it for example with BOOL or 1-byte bool
#define _bool_ __int32

// ��� ������ � ����� ���������� ���������� ���� �����,
// ���� ��� ������� ��-�������, ������ �������� _ptr_ �� ����� ������ ���, void* ��� int ��������
// ENG
// all addresses and the part of pointers are defined with this type,
// if another way is more convenient for you, you may replace _ptr_ with any other type, for example void* or int
typedef _dword_ _ptr_;



// �� ���� ���������� � ����������� ���������� ������ ���� ������������ - 4 �����
// ENG
// all the structures of library interface must have 4-byte alignment
#pragma pack(push, 4)


//��� "����������", ������������ ��� ������������ �������� Patcher::VarInit � Patcher::VarFind ��������.
// ENG
//"variable" type, it is used for values which are returned by Patcher::VarInit and Patcher::VarFind methods.
class Variable
{
public:
	// ���������� �������� '����������' (���������������� ���������)
	// ENG
	// returns the value of 'variable' (thread-safe reference)
	virtual _dword_ __stdcall GetValue() = 0;
	// ������������� �������� '����������' (���������������� ���������)
	// ENG
	// sets the value of 'variable' (thread-safe reference)
	virtual void __stdcall SetValue(_dword_ value) = 0;
	// ���������� ��������� �� �������� (��������� � �������� ����� ��������� �����������������)
	// ENG
	// returns a pointer to the value (reference to the value via this pointer is not thread-safe)
	virtual _dword_* __stdcall GetPValue() = 0;
};

// ��� '������� ������', ������ - 32 ���� 
// ������������ � HookContext
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

// ��������� HookContext
// ������������ � �������� ����������� �� LoHook ����
// ENG
// HookContext structure
// it is used in functions triggered by LoHook hook
struct HookContext
{
	int eax; //������� EAX, ������/���������; ENG: EAX register, read/modify
	int ecx; //������� ECX, ������/���������; ENG: ECX register, read/modify
	int edx; //������� EDX, ������/���������; ENG: EDX register, read/modify
	int ebx; //������� EBX, ������/���������; ENG: EBX register, read/modify
	int esp; //������� ESP, ������/���������; ENG: ESP register, read/modify
	int ebp; //������� EBP, ������/���������; ENG: EBP register, read/modify
	int esi; //������� ESI, ������/���������; ENG: ESI register, read/modify
	int edi; //������� EDI, ������/���������; ENG: EDI register, read/modify

	_ptr_ return_address; //����� ��������, ������/���������; ENG: return address, read/modify

	FlagsRegister flags; //������� ������, ������/���������; ENG: flag register, read/modify
	// ��� ������ ���������������� �� �������������� ������� ���� (����. delphi)
	// flags ����� ���� ��������� ��� _dword_ ����.
	// ENG
	// for programming languages that does not support bit field (i. e. delphi)
	// flags can be defined with _dword_ type.


	// ������� Push ����� ����������� �������� ������� ���������� PUSH ��� ��������� LoHook ����
	// ��� ������������� � ���������� ���� �������������� � ������� WriteLoHook ��� CreateLoHook
	// ������ ������ ������� ����� ���� �������� � ���� � ������� ���� ������� ��������� 128 �������.
	// ��� ������������� � ���������� ���� �������������� � ������� WriteLoHookEx ��� CreateLoHookEx
	// ���� ������ ��������������� ����������� ��� ������ WriteLoHookEx ��� CreateLoHookEx.
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

	// ������� Pop ����� ����������� �������� ������� ���������� POP ��� ��������� LoHook ����
	// ENG
	// Pop function has the similar effect as CPU command POP for LoHook hook context
	inline int Pop()
	{
		int r = *(int*)(esp);
		esp += 4;
		return r;
	}
};



// �������� ������������ �������� ������������� �� LoHook ����
// ENG
// values that may be returned by the function triggered by LoHook hook
#define EXEC_DEFAULT 1
#define NO_EXEC_DEFAULT 0
#define SKIP_DEFAULT 0


// �������� ������������ Patch::GetType()
// ENG
// values that may be returned by Patch::GetType()
#define PATCH_  0
#define LOHOOK_ 1
#define HIHOOK_ 2


// �������� ������������ PatcherInstance::Write() � PatcherInstance::CreatePatch()
// ENG
// values that are transfered to PatcherInstance::Write() and PatcherInstance::CreatePatch()
#define DATA_ 0
#define CODE_ 1


// ����������� ����� Patch
// ������� ��������� ����� �
// ������� ������� ������ PatcherInstance
// ENG
// Abstract class Patch
// instance may be created
// with PatcherInstance class methods
class Patch
{
public:
	// ���������� ����� �� �������� ��������������� ����
	// ENG
	// returns the address on which patch is placed
	virtual _ptr_ __stdcall GetAddress() = 0;

	// ���������� ������ �����
	// ENG
	// returns the size of the patch
	virtual _dword_ __stdcall GetSize() = 0;

	// ���������� ���������� ��� ���������� PatcherInstance, � ������� �������� ��� ������ ����
	// ENG
	// returns the unique name of PatcherInstance instance, which the patch was created via
	virtual char* __stdcall GetOwner() = 0;

	// ���������� ��� �����
	// ��� �� ���� ������ PATCH_
	// ��� LoHook ������ LOHOOK_
	// ��� HiHook ������ HIHOOK_
	// ENG
	// returns the type of the patch
	// for not-hook it is always PATCH_
	// for LoHook it is always LOHOOK_
	// for HiHook it is always HIHOOK_
	virtual int  __stdcall GetType() = 0;

	// ���������� true, ���� ���� �������� � false, ���� ���.
	// ENG
	// returns true if the patch is applied and false if not
	virtual _bool_ __stdcall IsApplied() = 0;

	// ��������� ���� 
	// ���������� >= 0 , ���� ����/��� ���������� �������
	// (������������ �������� �������� ���������� ������� ����� � ������������������
	// ������, ����������� �� ����������� ������� ������, ��� ������ �����, 
	// ��� ������� ��� �������� ����)
	// ���������� -2, ���� ���� ��� ��������
	// ��������� ���������� ������ ��������������� ������� � ���
	// ENG
	// applies the patch 
	// returns the value >= 0 if the patch/hook was applied successfully
	// (return value is the ordinal number of the patch in the sequence
	// of patches that are applied at (near) this address, the greater value,
	// the later the patch was applied)
	// returns -2 if the patch is already applied
	// The result of this method call is written to log in detail
	virtual _bool_ __stdcall Apply() = 0;

	// ApplyInsert ��������� ���� � ��������� ����������� ������ �
	// ������������������ ������, ����������� �� ����� ������.
	// ������������ �������� ���������� ��������������� � Patch::Apply
	// ������� ApplyInsert ����� ���������� �������� ��������, ������������ 
	// �������� Undo, ����� ��������� ���� � �� �� �����, �� ������� ��� ��� �� ������.
	// ENG
	// ApplyInsert applies the patch with the specific order in
	// the sequence of patches applied to this address.
	// return values are similar as Patch::Apply return values
	// you can give the value returned by Undo function to ApplyInsert 
	// to apply the patch at the place of the patch cancelled by Undo.
	virtual _bool_ __stdcall ApplyInsert(int zorder) = 0;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// ����� Undo
	// �������� ����(���) (� ������ ���� ���� �������� ��������� - ��������������� �������� ���)
	// ���������� ����� >= 0, ���� ���� (���) ��� ������� ������� 
	// (������������ �������� �������� ������� ����� � ������������������
	// ������, ����������� �� ������� ������, ��� ������ �����, 
	// ��� ������� ��� �������� ����)
	// ���������� -2, ���� ���� � ��� ��� ��� ������� (�� ��� ��������)
	// ��������� ���������� ������ ��������������� ������� � ���
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
	// ����� Destroy
	// �������� � ������������ ���������� ����/���
	// ���������� ������ 1 (��� ������������� � ����� ������� �������� ����������)
	// ��������� ����������� ��������������� ������� � ���
	// ENG
	// Destroy method
	// Cancels and irretrievably destroys th patch/hook
	// always returns 1 (for compatibility with ��� the earlier versions of the library)
	// The result of the destruction is written to log in detail
	virtual _bool_ __stdcall Destroy() = 0;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// ����� GetAppliedBefore
	// ���������� ���� ����������� ����� ������
	// ���������� NULL ���� ������ ���� �������� ������
	// ENG
	// GetAppliedBefore method
	// returns the patch applied before this one
	// returns NULL if this patch was the first of applied ones
	virtual Patch* __stdcall GetAppliedBefore() = 0;

	/////////////////////////////////////////////////////////////////////////////////////////////////////
	// ����� GetAppliedAfter
	// ���������� ���� ����������� ����� �������
	// ���������� NULL ���� ������ ���� �������� ���������
	// ENG
	// GetAppliedAfter method
	// returns the patch aplied after this one
	// returns NULL if this patch was the last of applied ones
	virtual Patch* __stdcall GetAppliedAfter() = 0;

};


// ����������� ����� LoHook (����������� �� Patch, �.�. �� ���� ���-��� �������� ������)
// ������� ��������� ����� �
// ������� ������� ������ PatcherInstance
// ENG
// Abstract class LoHook (the heir of Patch, so the LoHook is Patch in fact)
// instance may be created
// with PatcherInstance class methods
class LoHook : public Patch
{
};



// ��� �������, ���������� �������������� (LoHook) �����.
// ENG
// Low-level hook (LoHook) triggered function type.
typedef int(__stdcall* _LoHookFunc_)(LoHook*, HookContext*);

// �������� ������������ ��� �������� hooktype � PatcherInstance::WriteHiHook � PatcherInstance::CreateHiHook
// ENG
// the values for hooktype argument in PatcherInstance::WriteHiHook and PatcherInstance::CreateHiHook
#define CALL_ 0
#define SPLICE_ 1
#define FUNCPTR_ 2

// �������� ������������ ��� �������� subtype � PatcherInstance::WriteHiHook � PatcherInstance::CreateHiHook
// ENG
// the values for subtype argument in PatcherInstance::WriteHiHook and PatcherInstance::CreateHiHook
#define DIRECT_  0
#define EXTENDED_ 1
#define SAFE_  2

// �������� ������������ ��� �������� calltype � PatcherInstance::WriteHiHook � PatcherInstance::CreateHiHook
// the values for calltype argument in PatcherInstance::WriteHiHook and PatcherInstance::CreateHiHook
#define ANY_  0
#define STDCALL_ 0
#define THISCALL_ 1
#define FASTCALL_ 2 
#define CDECL_  3
#define FASTCALL_1 1 

// ����������� ����� HiHook (����������� �� Patch, �.�. �� ���� ���-��� �������� ������)
// ������� ��������� ����� � ������� ������� ������ PatcherInstance
// ENG
// Abstract class HiHook (the heir of Patch, so the LoHook is Patch in fact)
// instance may be created with PatcherInstance class methods
class HiHook : public Patch
{
public:
	// ���������� ��������� �� ������� (�� ���� � ������� � ������ SPLICE_),
	// ���������� �����
	// ��������! ������� ������� ��� �� ������������ ����, ����� ��������
	// ������������ (�� �������) ��������.
	// ENG
	// returns the pointer to the function (the bridge to the function in case of SPLICE_)
	// which is replaced with hook
	// Attention! Calling this function for hook which is not applied you can get
	// not relevant (but working) value
	virtual _ptr_ __stdcall GetDefaultFunc() = 0;

	// ���������� ��������� �� ������������ ������� (�� ���� � ������� � ������ SPLICE_),
	// ���������� ����� (������) �� ������� ������
	// (�.�. ���������� GetDefaultFunc() ��� ������� ������������ ���� �� ������� ������)
	// ��������! ������� ������� ��� �� ������������ ����, ����� ��������
	// ������������ (�� �������) ��������.
	// ENG
	// returns the pointer to the original function (the bridge to the function in case of SPLICE_)
	// which is replaced with hook (hooks) at this address
	// (i. e. returns GetDefaultFunc() for the first hook applied at this address)
	// Attention! Calling this function for hook which is not applied you can get
	// not relevant (but working) value
	virtual _ptr_ __stdcall GetOriginalFunc() = 0;

	// ���������� ����� �������� � ������������ ���
	// ����� ������������ ������ ���-�������
	// SPLICE_ EXTENDED_ ��� SAFE_ ����, ����� ������ ������ ��� ���� �������
	// ��� SPLICE_ DIRECT_ ���� ������� ���������� ������ 0 (�.�. ��� DIRECT_ ���� ����������� ������ ����� �������� ����� ��� - ���)
	// ENG
	// returns return to the original code address
	// can be used inside the hook-function
	// SPLICE_ EXTENDED_ or SAFE hook for getting know where it was called from
	// for SPLICE_ DIRECT_ hook tis function always returns 0 (i. e. there is no way for DIRECT_ hook to get to know the return address through it)
	virtual _ptr_ __stdcall GetReturnAddress() = 0;


	//# ver 2.1
	// ENG
	//# ver 2.1

	// ������������� �������� ���������������� ������ ����
	// ENG
	// set the value of hook user data
	virtual void __stdcall SetUserData(_dword_ data) = 0;
	// ���������� �������� ���������������� ������ ����
	// ���� �� ������ �������������, �� ����� 0
	// ENG
	// returns the value of hook user data
	// if the data was not set by user the function returns 0
	virtual _dword_ __stdcall GetUserData() = 0;
};




// ����������� ����� PatcherInstance
// �������/�������� ��������� ����� � ������� ������� CreateInstance � GetInstance ������ Patcher
// ��������������� ��������� ���������/������������� ����� � ���� � ���,
// �������� �� � ������ ���� ������/�����, ��������� ����������� patcher_x86.dll
// Abstract class PatcherInstance
// instance may be created/got with CreateInstance or GetInstance methods of Patcher class
// this class directly allows to create/apply patches and hooks to code,
// adding them to common tree of patches/hooks created with patcher_x86.dll library
class PatcherInstance
{
public:
	////////////////////////////////////////////////////////////
	// ����� WriteByte
	// ����� ������������ ����� �� ������ address
	// (������� � ��������� DATA_ ����)
	// ���������� ��������� �� ����
	// ENG
	// WriteByte method
	// writes a 1-byte number at the specified address
	// (creates and applies DATA_ patch)
	// Returns a pointer to the patch
	virtual Patch* __stdcall WriteByte(_ptr_ address, int value) = 0;

	////////////////////////////////////////////////////////////
	// ����� WriteWord
	// ����� ������������ ����� �� ������ address
	// (������� � ��������� DATA_ ����)
	// ���������� ��������� �� ����
	// ENG
	// WriteWord method
	// writes a 2-byte number at the specified address
	// (creates and applies DATA_ patch)
	// Returns a pointer to the patch
	virtual Patch* __stdcall WriteWord(_ptr_ address, int value) = 0;

	////////////////////////////////////////////////////////////
	// ����� WriteDword
	// ����� ��������������� ����� �� ������ address
	// (������� � ��������� DATA_ ����)
	// ���������� ��������� �� ����
	// ENG
	// WriteDword method
	// writes a 4-byte number at the specified address
	// (creates and applies DATA_ patch)
	// Returns a pointer to the patch
	virtual Patch* __stdcall WriteDword(_ptr_ address, int value) = 0;

	////////////////////////////////////////////////////////////
	// ����� WriteJmp
	// ����� jmp to ����� �� ������ address
	// (������� � ��������� CODE_ ����)
	// ���������� ��������� �� ����
	// ���� ��������� ����� ���������� �������,
	// �.�. ������ ����� >= 5, ������� ����������� NOP'���.
	// ENG
	// WriteJmp method
	// writes 'jmp to' opcode at the specified address
	// (creates and applies CODE_ patch)
	// Returns a pointer to the patch
	// patch replaces an integer number of opcodes,
	// i. e. if the size of the patch is >= 5 then the gap is filled with NOPs
	virtual Patch* __stdcall WriteJmp(_ptr_ address, _ptr_ to) = 0;

	////////////////////////////////////////////////////////////
	// ����� WriteHexPatch
	// ����� �� ������ address ������������������ ����,
	// ������������ hex_str
	// (������� � ��������� DATA_ ����)
	// ���������� ��������� �� ����
	// hex_str - ��-������ ����� ��������� ����������������� �����
	// 0123456789ABCDEF (������ ������� �������!) ��������� ������� 
	// ��� ������ ������� hex_str ������������(������������)
	// ������ ������������ � �������� ��������� ����� ������
	// ������������� � ������� Binary copy � OllyDbg
	/* ������:
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
	// ����� WriteCodePatchVA
	// � ������������ ���� ���������� ������ �� ��������������,
	// �������� (����) �������� ������-�������� WriteCodePatch
	// ENG
	// WriteCodePatchVA method
	// this method is not supposed to be used in the original form
	// see the description of the shell method WriteCodePatch (below)
	virtual Patch* __stdcall WriteCodePatchVA(_ptr_ address, char* format, _dword_* va_args) = 0;

	////////////////////////////////////////////////////////////
	// ����� WriteLoHook
	// ������� �� ������ address �������������� ��� (CODE_ ����) � ��������� ���
	// ���������� ��������� �� ���
	// func - ������� ���������� ��� ������������ ����
	// ������ ����� ��� _LoHookFunc_: int __stdcall func(LoHook* h, HookContext* c);
	// � HookContext* c ���������� ��� ������/��������� 
	// �������� ���������� � ����� ��������
	// ���� func ���������� EXEC_DEFAULT, �� 
	// ����� ���������� func ����������� �������� ����� ���.
	// ���� - SKIP_DEFAULT - �������� ��� �� �����������
	//
	// ��������! 
	// ������ ������, ������� ����� ���� �������� � ���� ���������
	// � ������� ������������� c->esp � �->Push, ��������� 128 �������.
	// ���� ��������� ���� ����������� ����������� ����� WriteLoHookEx ��� CreateLoHookEx.
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
	// ����� WriteHiHook
	// ������� �� ������ address ��������������� ��� � ��������� ���
	// ���������� ��������� �� ���
	//
	// new_func - ������� ���������� ������������
	//
	// hooktype - ��� ����:
	//  CALL_ -  ��� �� ����� ������� �� ������ address
	//     �������������� ������ E8 � FF 15, � ��������� ������� ��� �� ���������������
	//     � � ��� ������� ���������� �� ���� ������
	//  SPLICE_ - ��� ��������������� �� ���� ������� �� ������ address
	//  FUNCPTR_ - ��� �� ������� � ��������� (����������� �����, � �������� ��� ����� � �������� �������)
	//
	// subtype - ������ ����:
	//  DIRECT_ - new_func ����� ��� �� ��� ��� �
	//     ������������ ���������� �������
	//     ����������: ������ __thiscall f(this) 
	//        ����� ������������ __fastcall f(this_)
	//        ������ __thiscall f(this, ...)  ����� ������������ 
	//        __fastcall f(this_, no_used_edx, ...) 
	//  EXTENDED_ - ������� new_func ������ �������� ���������� ����������
	//     ��������� �� ��������� HiHook �, � ������ 
	//     ���������� �������� ������� __thiscall � __fastcall
	//     ����������� ��������� ���������� ��������� ������� 
	// ����� ������� ������� new_func ������ ����� ���
	// ? __stdcall new_func(HiHook* hook, ?) ��� ? ? orig(?)
	// ��� ? __cdecl orig(?) ����������� ����� ������������� �������,
	// ����������� ��� ? __cdecl new_func(HiHook* hook, ?)
	// (��� ������������� � ����� ������� �������� ����������)
	//
	// ��������! EXTENDED_ FASTCALL_ ������������ ������ ������� � 2-�� � ����� �����������
	// ��� __fastcall c 1 ���������� ����������� EXTENDED_ FASTCALL_1 / EXTENDED_ THISCALL_
	//
	//   SAFE_ - �� �� ����� ��� � EXTENDED_, ������ ����� ������� (�� ����� ������) GetDefaultFunc() �����������������
	//    �������� ��������� ���������� EAX, ECX (���� �� FASTCALL_ � �� THISCALL_),
	//    EDX (���� �� FASTCALL_), EBX, ESI, EDI, ������ �� ������ ������ ���������� �������
	//
	//  � ����������� ����������� ������� ������� ������������ EXTENDED_
	//  �� DIRECT_ ����������� ������� ��-�� ���������� ����� � ����� ���������� �������
	//
	// calltype - ���������� � ������ ������������ ���������� �������:
	//  STDCALL_
	//  THISCALL_
	//  FASTCALL_
	//  CDECL_
	// ���������� ����� ��������� ���������� ��� ���� ����� EXTENDED_ ��� ���������
	// �������� ���� � ����� ���������� �������
	//
	// CALL_, SPLICE_ ��� �������� CODE_ ������
	// FUNCPTR_ ��� �������� DATA_ ������
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
	// ������ Create...
	// ������� ����/��� ��� �� ��� � ��������������� ������ Write...,
	// �� �� ��������� ���
	// ���������� ��������� �� ����/���
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
	// ����� ApplyAll
	// ��������� ��� �����/����, ��������� ���� ����������� PatcherInstance
	// ������ ���������� 1 (��� ������������� � ����� ������� �������� ����������)
	// (��. Patch::Apply)
	// ENG
	// ApplyAll method
	// applies all the patches/hooks created with this PatcherInstance instance
	// always returns 1 (for the compatibility with the previous versions of the library)
	// (see Patch::Apply)
	virtual _bool_ __stdcall ApplyAll() = 0;

	////////////////////////////////////////////////////////////
	// ����� UndoAll
	// �������� ��� �����/����, ��������� ���� ����������� PatcherInstance
	// �.�. ��� ������� �� ������/����� �������� ����� Undo
	// ������ ���������� 1 (��� ������������� � ����� ������� �������� ����������)
	// (��. Patch::Undo)
	// ENG
	// UndoAll method
	// cancels all the patches/hooks created with this PatcherInstance instance
	// i. e. calls Undo method for each patch/hook
	// always returns 1 (for the compatibility with the previous versions of the library)
	// (see Patch::Undo)
	virtual _bool_ __stdcall UndoAll() = 0;

	////////////////////////////////////////////////////////////
	// ����� DestroyAll
	// �������� � ������������ ���������� ��� �����/����, ��������� ���� ����������� PatcherInstance
	// �.�. ��� ������� �� ������/����� �������� ����� Destroy
	// ������ ���������� 1 (��� ������������� � ����� ������� �������� ����������)
	// (��. Patch::Destroy)
	// ENG
	// DestroyAll method
	// cancels and irretrievably destroys all the patches/hooks created with this PatcherInstance instance
	// i. e. calls Destroy method for each patch/hook
	// always returns 1 (for the compatibility with the previous versions of the library)
	// (see Patch::Destroy)
	virtual _bool_ __stdcall DestroyAll() = 0;

	// � ������������ ���� ���������� ������ �� ��������������,
	// �������� (����) �������� ������-�������� WriteDataPatch
	// ENG
	// this method is not supposed to be used in the original form
	// see the description of the shell method WriteDataPatch (below)
	virtual Patch* __stdcall WriteDataPatchVA(_ptr_ address, char* format, _dword_* va_args);
	// � ������������ ���� ���������� ������ �� ��������������,
	// �������� (����) �������� ������-�������� CreateDataPatch
	// ENG
	// this method is not supposed to be used in the original form
	// see the description of the shell method CreateDataPatch (below)
	virtual Patch* __stdcall CreateDataPatchVA(_ptr_ address, char* format, _dword_* va_args);


	// ����� GetLastPatchAt
	// ���������� NULL, ���� � ����������� ������ address �� ��� �������� �� ���� ����/���,
	// ��������� ������ ����������� PatcherInstance
	// ����� ���������� ��������� ����������� ����/��� � ����������� ������ address,
	// ��������� ������ ����������� PatcherInstance
	// ENG
	// GetLastPatchAt method
	// returns NULL if no patch/hook created with this PatcherInstance instance
	// was applied at (near) the specified address
	// otherwise returns the last patch/hook created with this PatcherInstance instance
	// and applied at (near) the specified address
	virtual Patch* __stdcall GetLastPatchAt(_ptr_ address) = 0;

	// ����� UndoAllAt
	// �������� ����� ����������� ������ ����������� PatcherInstance
	// � ����������� ������ address 
	// ������ ���������� 1 (��� ������������� � ����� ������� �������� ����������)
	// (��. Patch::Undo)
	// ENG
	// UndoAllAt method
	// cancels all the patches/hooks created with this PatcherInstance instance
	// applied at (near) the specified address
	// always returns 1 (for the compatibility with the previous versions of the library)
	// (see Patch::Undo)
	virtual _bool_ __stdcall UndoAllAt(_ptr_ address) = 0;

	// ����� GetFirstPatchAt
	// ���������� NULL, ���� � ����������� ������ address �� ��� �������� �� ���� ����/���,
	// ��������� ������ ����������� PatcherInstance
	// ����� ���������� ������ ����������� ����/��� � ����������� ������ address,
	// ��������� ������ ����������� PatcherInstance
	// ENG
	// GetFirstPatchAt method
	// returns NULL if no patch/hook created with this PatcherInstance instance
	// was applied at (near) the specified address
	// otherwise returns the first patch/hook created with this PatcherInstance instance
	// and applied at (near) the specified address
	virtual Patch* __stdcall GetFirstPatchAt(_ptr_ address) = 0;


	////////////////////////////////////////////////////////////
	// ����� Write
	// ����� �� ������ address ������/��� �� ������ �� ������ data �������� size ���� 
	// ���� is_code == 1, �� ��������� � ������� CODE_ ����, ���� 0 - DATA_ ����.
	// ���������� ��������� �� ����
	// Write method
	// writes data/code from memory at 'data' to memory at 'address' with the specified size
	// if is_code == 1 then CODE_ patch is created and applied, if is_code == 0 then DATA_ patch is created and applied.
	// Returns a pointer to the patch
	virtual Patch* __stdcall Write(_ptr_ address, _ptr_ data, _dword_ size, _bool_ is_code = 0) = 0;
	///////////////////////////////////////////////////////////////////
	// ����� CreatePatch
	// ������ ���� ��� �� ��� � ����� Write,
	// �� �� ��������� ���
	// ���������� ��������� �� ����
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
	// ����� WriteLoHookEx
	// ���������� ������ WriteLoHook, �� ����� �������������� ��������
	// stack_delta - ������ ������ ������� ����� ��������� � ���� ���������
	// ��������� HookContext::esp � HookContext::Push ������ func.
	//
	// ���������� ��������� �� LoHook ���
	// ENG
	// WriteLoHookEx method
	// this method is similar to WriteLoHook, but has an additional argument
	// stack_delta - the size of data which can be placed to the stack of the context
	// via HookContext::esp and HookContext::Push inside func.
	//
	// Returns a pointer to the LoHook hook
	virtual LoHook* __stdcall WriteLoHookEx(_ptr_ address, _LoHookFunc_ func, _dword_ stack_delta) = 0;
	// ����� CreateLoHookEx
	// ������� ��� ��� �� ��� � WriteLoHookEx,
	// �� �� ��������� ���.
	// ���������� ��������� �� LoHook ���
	// ENG
	// CreateLoHookEx method
	// creates the same hook as WriteLoHookEx method,
	// but DOES NOT APPLY it
	// returns a pointer to the LoHook hook
	virtual LoHook* __stdcall CreateLoHookEx(_ptr_ address, _LoHookFunc_ func, _dword_ stack_delta) = 0;


	// � ������������ ���� ���������� ������ �� ��������������,
	// �������� (����) �������� ������-�������� WriteHexHook
	// ENG
	// this method is not supposed to be used in the original form
	// see the description of the shell method WriteHexHook (below)
	virtual LoHook* __stdcall WriteHexHookVA(_ptr_ address, _bool_ exec_default, char* hex_str, _dword_* va_args) = 0;
	// � ������������ ���� ���������� ������ �� ��������������,
	// �������� (����) �������� ������-�������� CreateHexHook
	// ENG
	// this method is not supposed to be used in the original form
	// see the description of the shell method CreateHexHook (below)
	virtual LoHook* __stdcall CreateHexHookVA(_ptr_ address, _bool_ exec_default, char* hex_str, _dword_* va_args) = 0;



	// ����� BlockAt ������������� ���� �� ������������ ����� (������ �� ���������� ����� � �� �� �����������)
	// ��� ������� ���������� PatcherInstance
	// ����� ���� ������ ��������� PatcherInstance �� ����� ��������� 
	// ����� �� ����� ������
	// ENG
	// BlockAt method locks the specified address (exactly address, not area near it)
	// for this PatcherInstance instance
	// after this the PatcherInstance instance cannot apply
	// patches at this address
	virtual void __stdcall BlockAt(_ptr_ address) = 0;


	//## ver 2.6
	// ENG
	//## ver 2.6

	// ����� BlockAllExceptVA ������������� ���� �� ��� ������ ����� ��������� � va_args
	// (��� �� ��� � ����� BlockAt ��������� ����������� �������� � �� �������������)
	// ��� ������� ���������� PatcherInstance
	// ����� ���� ������ ��������� PatcherInstance �� ����� ��������� 
	// ����� �� ���� ������� ����� ���������.
	// ������ ������� � va_args ������ ������������� 0 (�����)
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

	// � ������������ ���� ���������� ������ �� ��������������,
	// �������� (����) �������� ������-�������� WriteAsmPatch
	// ENG
	// this method is not supposed to be used in the original form
	// see the description of the shell method WriteAsmPatch (below)
	virtual Patch* __stdcall WriteAsmPatchVA(_ptr_ address, _dword_* va_args) = 0;

	// � ������������ ���� ���������� ������ �� ��������������,
	// �������� (����) �������� ������-�������� CreateAsmPatch
	// ENG
	// this method is not supposed to be used in the original form
	// see the description of the shell method CreateAsmPatch (below)
	virtual Patch* __stdcall CreateAsmPatchVA(_ptr_ address, _dword_* va_args) = 0;

	// � ������������ ���� ���������� ������ �� ��������������,
	// �������� (����) �������� ������-�������� WriteAsmHook
	// ENG
	// this method is not supposed to be used in the original form
	// see the description of the shell method WriteAsmHook (below)
	virtual LoHook* __stdcall WriteAsmHookVA(_ptr_ address, _dword_* va_args) = 0;

	// � ������������ ���� ���������� ������ �� ��������������,
	// �������� (����) �������� ������-�������� CreateAsmHook
	// ENG
	// this method is not supposed to be used in the original form
	// see the description of the shell method CreateAsmHook (below)
	virtual LoHook* __stdcall CreateAsmHookVA(_ptr_ address, _dword_* va_args) = 0;

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// WriteAsmPatch ����� ���� �� ������ address
	// ���������� ��������� �� ����
	// ��������� ... - ��� ������, ���������� ��� �� ����������
	// ������������ ��� ����������, �������������� OllyDbg 1.04 (������ �� MMX � amd 3DNow! ������������)
	// ��������! � ������� �� OllyDbg, ����� ����� �� ������� �������� 0x ��� ��������� h �������� ��� ������������!
	// ������� �� ��������� ������ ���������������� ����� ����
	// � ����� ������ ����� ���� ��������� ����������, ����������� �������� ';'
	// ��� �� ���������� ����� ��������� �����;
	// ���������� ����� - label_name: (��� �����, ���������),
	// ������������� label_name (��� ��� ���������);
	// ������������ ����� ����� ����� - 39 ��������, ��� ����� ��������� �����, �����, ������� '_' � '.';
	// ��� ����� ��������� ���������������� times (����� �������� ���������� ��� ��������� ����������)
	// ��������, ����������� "times 3 nop" ����� ��� 90 90 90
	// ��� ����� ��������� ���������������� _ExecCopy <�����>, <������> (����� ��� �� ������ �� ������ <�����> �������� <������>)
	// ��� ����� ��������� ���������������� db <�����>, dw <�����>, dd <����� ��� �����>.
	// ������ ����� ��������� ������-������� %d. � ���� ������ �� ������� ������ ��������� ��������������� ���������� ��������������� ����� (��������/�����������/������/...)
	// ��������! ��������� ���������� (�������) ������ ����������� ���� 0 (NULL)!
	// ����������� ������: WriteAsmPatch(0x112233, "begin: call %d", MyFunc, "jmp begin", "jne dword [%d]", 0xAABBCC, 0);
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
	// ����� CreateAsmPatch
	// ������� ���� ��� �� ��� � ����� WriteAsmPatch,
	// �� �� ��������� ���
	// ���������� ��������� �� ����
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
	// WriteAsmHook ����� �� ������ address ����������� ���
	// � ������ LoHook ��� ������ ��������������� �������.
	// ���� ���� ������� ����� � ������ CreateAsmHook ��� WriteAsmHook
	// ����� �� ������� ��� ������� ���� � ������� WriteAsmPatch (��. WriteAsmPatch)
	// � ������� ������ WriteAsmPatch ����� ��� ����� ��������� (� ����� �� ���������) ������������� _ExecDefault,
	// ������� ��������� �������� ����� ���
	// ��������! � ���� �� ����� ���� ������ ����� ������������� _ExecDefault
	// ���������� ��������� �� LoHook ���
	// ��������! ��������� ���������� (�������) ������ ����������� ���� 0 (NULL)!
	// ����������� ������: WriteAsmHook(0x112233, "cmp eax, 0; jne SkipDefault; _ExecDefault; jmp End; SkipDefault: mov ecx, 2; End: retn", 0);
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
	// ����� CreateAsmHook
	// ������� ��� ��� �� ��� � WriteAsmHook,
	// �� �� ��������� ���.
	// ���������� ��������� �� LoHook ���
	// ENG
	// CreateAsmHook method
	// creates the same hook as WriteAsmHook method,
	// but DOES NOT APPLY it
	// returns a pointer to the LoHook hook
	inline LoHook* CreateAsmHook(_ptr_ address, ...)
	{
		return CreateAsmHookVA(address, (_dword_*)((_ptr_)&address + 4));
	}



	// WriteHexHook ����� �� ������ address ����� ����������� ���
	// � ������ LoHook ��� ������ ��������������� �������.
	// ���� ���� ������� ����� � ������ CreateHexHook ��� WriteHexHook
	// ����� �� ������� ��� ������� ���� � ������� WriteCodePatch (��. WriteCodePatch)
	//
	// exec_default - ��������� �� �������� ����� ��� ����� ���������� ���� ����
	// ���������� ��������� �� LoHook ���
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
	// ����� CreateHexHook
	// ������� ��� ��� �� ��� � WriteHexHook,
	// �� �� ��������� ���.
	// ���������� ��������� �� LoHook ���
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
	// ����� WriteCodePatch
	// ����� �� ������ address ������������������ ����,
	// ������������ format � ...
	// (������� � ��������� CODE_ ����)
	// ���������� ��������� �� ����
	// format - ��-������ ����� ��������� ����������������� �����
	// 0123456789ABCDEF (������ ������� �������!),
	// � ��� �� ����������� ������-������� (������ �������!):
	// %b - (byte) ����� ������������ ����� �� ...
	// %w - (word) ����� ������������ ����� �� ...
	// %d - (dword) ����� ��������������� ����� �� ...
	// %j - ����� jmp �� ����� �� ...
	// %� - ����� �all ...
	// %m - �������� ��� �� ������ ... �������� ... (�.�. ������ 2 ��������� �� ...)
	//      ����������� ���������� ����������� MemCopyCodeEx (��. ��������)
	// %% - ����� ������ � ������-��������� �� ... 
	// %o - (offset) �������� �� ������ �� ��������� �������� ������� �
	//      Complex ����, ������������ ������ Complex ����.
	//      (���� ������ ������ �� ��������� � ��� Complex ���)
	// %n - ����� nop ������, ����������� ������ ...                                  \
    #0: - #9: - ������������� ����� (�� 0 �� 9) � ������� ����� ������� � ������� #0 - #9                              \
    #0 -  #9  - ����� ������������ ����� ����� ������� EB, 70 - 7F, E8, E9, 0F80 - 0F8F
	//      ��������������� �����; ����� ������ ������� ������ �� �����
	// ~b - ����� �� ... ���������� ����� � ����� ������������� �������� �� ����
	//      �������� � 1 ���� (������������ ��� ������� EB, 70 - 7F)
	// ~d - ����� �� ... ���������� ����� � ����� ������������� �������� �� ����
	//      �������� � 4 ����� (������������ ��� ������� E8, E9, 0F 80 - 0F 8F)
	// %. - ������ �� ������ ( ��� � ����� ������ �� ����������� ���� ������ ����� % ) 
	// ����������� ������:
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
	// %� - writes �all ...
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
	// ����� CreateCodePatch
	// ������� ���� ��� �� ��� � ����� WriteCodePatch,
	// �� �� ��������� ���
	// ���������� ��������� �� ����
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
	// ����� WriteDataPatch
	// ����� �� ������ address ������������������ ����,
	// ������������ format � ...
	// (������� � ��������� DATA_ ����)
	// ���������� ��������� �� ����
	// format - ��-������ ����� ��������� ����������������� �����
	// 0123456789ABCDEF (������ ������� �������!),
	// � ��� �� ����������� ������-������� (������ �������!):
	// %b - (byte) ����� ������������ ����� �� ...
	// %w - (word) ����� ������������ ����� �� ...
	// %d - (dword) ����� ��������������� ����� �� ...
	// %m - �������� ������ �� ������ ... �������� ... (�.�. ������ 2 ��������� �� ...)
	// %% - ����� ������ � ������-��������� �� ... 
	// %o - (offset) �������� �� ������ �� ��������� �������� ������� �
	//      Complex ������, ������������ ������ Complex ������.
	//      (���� ������ ������ �� ��������� � ���� Complex ������)
	// %. - ������ �� ������ ( ��� � ����� ������ �� ����������� ���� ������ ����� % ) 
	// ����������� ������:
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
	// ����� CreateDataPatch
	// ������� ���� ��� �� ��� � ����� WriteDataPatch,
	// �� �� ��������� ���
	// ���������� ��������� �� ����
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

// ����� Patcher
// ENG
// Patcher class
class Patcher
{
public:
	// �������� ������:
	// ENG
	// main methods:

	///////////////////////////////////////////////////
	// ����� CreateInstance
	// ������� ��������� ������ PatcherInstance, ������� 
	// ��������������� ��������� ��������� ����� � ���� �
	// ���������� ��������� �� ���� ���������.
	// owner - ���������� ��� ���������� PatcherInstance
	// ����� ���������� NULL, ���� ��������� � ������ owner ��� ������
	// ���� owner == NULL ��� owner == "" �� 
	// ��������� PatcherInstance ����� ������ � ������ ������ ��
	// �������� ���� ������� �������.
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
	// ����� GetInstance
	// ���������� ��������� �� ��������� PatcherInstance
	// � ������ owner.
	// ����� ���������� NULL � ������, ���� 
	// ��������� � ������ owner �� ���������� (�� ��� ������)
	// � �������� ��������� ����� ���������� ��� ������.
	// ������������ ��� :
	// - �������� ������� �� ��������� ���, ������������ patcher_x86.dll
	// - ��������� ������� �� ���� ������ � ����� ���������� ����,
	//   ������������� patcher_x86.dll
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
	// ����� GetLastPatchAt
	// ���������� NULL, ���� � ����������� ������ address �� ��� �������� �� ���� ����/���
	// ����� ���������� ��������� ����������� ����/��� � ����������� ������ address
	// ����� ��������������� �������� �� ���� ������ � �������� ����������� 
	// ��������� ���� ����� � Patch::GetAppliedBefore
	// ENG
	// GetLastPatchAt method
	// returns NULL if no patch/hook was applied at (near) the specified address
	// otherwise returns the last patch/hook applied at (near) the specified address
	// it is possible to go through all the patches at (near) the specific address
	// using this method and Patch::GetAppliedBefore
	virtual Patch* __stdcall GetLastPatchAt(_ptr_ address);

	///////////////////////////////////////////////////
	// ����� UndoAllAt
	// �������� ��� �����/���� � ����������� ������ address
	// ������ ���������� 1 (��� ������������� � ����� ������� �������� ����������)
	// ENG
	// UndoAllAt method
	// cancels all the patches/hooks applied at (near) the specified address
	// always returns 1 (for the compatibility with the previous versions of the library)
	virtual Patch* __stdcall UndoAllAt(_ptr_ address);

	///////////////////////////////////////////////////
	// ����� SaveDump
	// ��������� � ���� � ������ file_name:
	// - ���������� � ����� ���� ����������� PatcherInstance
	// - ���������� ���� ����������� ������/�����
	// - ������ ���� ����������� ������ � ����� � �� �������� ���������, ���������, ���������� ������������ ����������, �������������� (������� PatcherInstance)
	// ENG
	// SaveDump method
	// save the information to file with name file_name:
	// - the amount and the names of each of PatcherInstance instances
	// - the amount of all the patches/hooks applied
	// - the list of all the patches/hooks applied with their addresses, sizes, global apply order, owners (PatcherInstance names)
	virtual void __stdcall SaveDump(char* file_name) = 0;

	///////////////////////////////////////////////////
	// ����� SaveLog
	// ��������� � ���� � ������ file_name ��� 
	// ���� ����������� ��������� � ���� ����� 0 �������.
	// �������� ����������� ����� ������ � ���������� ����������
	// ��������� ���� patcher_x86.ini c ����������: Logging = 1
	// ENG
	// SaveLog method
	// save the log to file with name file_name:
	// if logging is disable the log will contain 0 records.
	// you can enable logging by creating patcher_x86.ini in the folder of library 
	// with the record: Logging = 1
	virtual void __stdcall SaveLog(char* file_name) = 0;

	///////////////////////////////////////////////////
	// ����� GetMaxPatchSize
	// ���������� patcher_x86.dll ����������� ��������� �����������
	// �� ������������ ������ �����,
	// ����� - ����� ������ � ������� ������ GetMaxPatchSize
	// (�� ������ ������ ��� 262144 ����, �.�. ������� :) )
	// ENG
	// GetMaxPatchSize method
	// The library patcher_x86.dll has some limits
	// on the maximum patch size,
	// you can get to know this limit with GetMaxPatchSize method call
	// (currently it is 262144 bytes, i. e. a lot :) )
	virtual int __stdcall GetMaxPatchSize() = 0;

	// �������������� ������:
	// ENG
	// additional methods:

	///////////////////////////////////////////////////
	// ����� WriteComplexDataVA
	// � ������������ ���� ���������� ������ �� ��������������,
	// �������� (����) �������� ������-�������� WriteComplexString
	// ENG
	// WriteComplexDataVA method
	// this method is not supposed to be used in the original form
	// see the description of the shell method WriteComplexString (below)
	virtual int __stdcall WriteComplexDataVA(_ptr_ address, char* format, _dword_* args) = 0;

	///////////////////////////////////////////////////
	// ����� GetOpcodeLength
	// �.�. ������������ ���� �������
	// ���������� ����� � ������ ������ �� ������ p_opcode
	// ���������� 0, ���� ����� ����������
	// ENG
	// GetOpcodeLength method
	// so-called opcode length disassembler
	// returns the length in bytes of an opcode from p_opcode address
	// returns 0 if an opcode is unknown
	virtual int __stdcall GetOpcodeLength(_ptr_ p_opcode) = 0;

	///////////////////////////////////////////////////
	// ����� MemCopyCode
	// �������� ��� �� ������ �� ������ src � ������ �� ������ dst
	// MemCopyCode �������� ������ ����� ���������� ������� �������� >= size. ������ �����������!
	// ���������� ������ �������������� ����.
	// ���������� ��������� �� �������� ����������� ������ ���,
	// ��� ��������� �������� ������ E8 (call), E9 (jmp long), 0F80 - 0F8F (j** long)
	// c ������������� ���������� �� ������ � ��� ������, ���� ���������� 
	// ���������� �� ������� ����������� ������.
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
	// ����� GetFirstPatchAt
	// ���������� NULL, ���� � ����������� ������ address �� ��� �������� �� ���� ����/���
	// ����� ���������� ������ ����������� ����/��� � ����������� ������ address
	// ����� ��������������� �������� �� ���� ������ � �������� ����������� 
	// ��������� ���� ����� � Patch::GetAppliedAfter
	// ENG
	// GetFirstPatchAt method
	// returns NULL if no patch/hook was applied at (near) the specified address
	// otherwise returns the first patch/hook applied at (near) the specified address
	// it is possible to go through all the patches at (near) the specific address
	// using this method and Patch::GetAppliedAfter
	virtual Patch* __stdcall GetFirstPatchAt(_ptr_ address);

	///////////////////////////////////////////////////
	// ����� MemCopyCodeEx
	// �������� ��� �� ������ �� ������ src � ������ �� ������ dst
	// ���������� ������ �������������� ����.
	// ���������� �� MemCopyCode ���,
	// ��� ��������� �������� ������ EB (jmp short), 70 - 7F (j** short)
	// c ������������� ���������� �� ������ � ��� ������, ���� ���������� 
	// ���������� �� ������� ����������� ����� (� ���� ������ ��� ���������� ��
	// ��������������� E9 (jmp long), 0F80 - 0F8F (j** long) ������.
	// ��������! ��-�� ����� ������ �������������� ���� ����� ��������� ����������� 
	// ������ �����������.
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

	// ����� VarInit
	// �������������� "����������" c ������ name � ������������� �������� "����������" ������ value
	// ���� "����������" � ����� ������ ��� ����������, �� ������ ������������� �� �������� ������ value
	// ���������� ��������� �� "����������" � ������ ������ � NULL � ��������� ������
	// ENG
	// VarInit method
	// initializes a "variable" with the specified name and set its value
	// if the "variable" with this name already exists then its value is just setted
	// returns the pointer to the "variable" in case of success or NULL otherwise
	virtual Variable* __stdcall VarInit(char* name, _dword_ value) = 0;
	// ����� VarFind
	// ���������� ��������� �� "����������" � ������ name, ���� ����� ���� ����������������
	// ���� ���, ���������� NULL
	// ENG
	// VarFind method
	// returns a pointer to the "variable" with the specified name, if it was initialized
	// otherwise returns NULL
	virtual Variable* __stdcall VarFind(char* name) = 0;


	// ver 2.6
	// ENG
	// ver 2.6

	// ����� PreCreateInstance
	// ������� ������������� ��������� PatcherInstance � ��������� ������.
	// PatcherInstance ��������� ����� ������� �� ����� ��������� �����.
	// ���� ������������� ��������� ������������ ��� ���������� ������� PatcherInstance::BlockAt � PatcherInstance::BlockAllExceptVA
	// ����� ����� ���� ������������� ������ �� ���� ��� ������ PatcherInstance ����� ���������� ������ � ������� CreateInstance
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

	// �������� � ����������...
	// ENG
	// the description is under construction...
	virtual int __stdcall WriteAsmCodeVA(_ptr_ address, _dword_* args) = 0;
	virtual _ptr_ __stdcall CreateCodeBlockVA(_dword_* args) = 0;


	// ����� VarGetValue ���������� �������� "����������" c ������ name
	// ���� "����������" � ����� ������ �� ���� ����������������, ���������� default_value
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

	// ����� VarValue ���������� ������ �� �������� "����������" c ������ name
	// ���� "����������" � ����� ������ �� ���� ����������������, �������������� �� � ������������� �������� ������ 0
	// ��������, ��������� � �������� ���������� �� ������ �����������������
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
	// ����� WriteComplexData
	// �������� ����� ������� �����������  
	// ������ WriteComplexDataVA
	// ���� ����� ��������� ����� � �� � ����������, �.�. ��� ��� 
	// ���������� � �� � �����
	// ���������� ������ ����� ��� �� ��� � � PatcherInstance::WriteCodePatch
	// (��. �������� ����� ������)
	// �� ���� ����� ����� �� ������ address, ������������������ ����,
	// ������������ ����������� format � ...,
	// ��! �� ������� ��������� ������ Patch, �� ����� ����������� (�.�. �� �������� �������� ������, �������� ������ � ������ �� ������� ���� � �.�.)
	// ��������!
	// ����������� ���� ����� ������ ��� ������������� �������� ������
	// ����, �.�. ������ ���� ������� ������ � ���� ������, 
	// � � ��� �������������� ��������� ������ � �������
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





// ��������������� ������������ ������ �������� � �������
// ENG
// restore class and structure members alignment
#pragma pack(pop)

//////////////////////////////////////////////////////////////////

// ������� GetPatcher
// ��������� ���������� �, � ������� ������ ������������ �������������� 
// ������� _GetPatcherX86@0, ���������� ��������� �� ������ Patcher,
// ����������� �������� �������� ���� ���������� ���������� patcher_x86.dll,
// ���������� NULL ��� �������
// ������� �������� 1 ���, ��� �������� �� �� �����������
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

