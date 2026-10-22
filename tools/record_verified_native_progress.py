"""Record reviewed, binary-traced native functions for the progress site.

This is a separate provenance lane from ReAgent batches. It only adds the
functions below after checking the SHA-verified PE and their source files.
Existing ReAgent decisions are never rewritten.
"""
from __future__ import annotations

import hashlib
import json
from datetime import datetime, timezone
from pathlib import Path

from audit_native_globals import bytes_at, read_pe
from disasm_original import UNPROTECTED_SHA256

ROOT = Path(__file__).resolve().parents[1]
STATE = ROOT / "reports/pipeline/state.json"
PE = ROOT.parent / "hp6ss/build/sdk-lab/pc/hp6.exe"

# Address, C++ symbol, source, build verification, remaining limitation.
FUNCTIONS = [
    ("0045df40", "command_return_one_0045df40", "src/presentation/NativeCommandLeafs.cpp", "compiled_only", "Returns AL=1; eight command-table entries share it."),
    ("0048ad30", "command_storage_popup_active_0048ad30", "src/presentation/NativeCommandLeafs.cpp", "compiled_only", "Reads the storage-popup state at +0x0C."),
    ("0048b020", "command_toggle_text_clip_0048b020", "src/presentation/NativeCommandLeafs.cpp", "compiled_only", "Toggles DAT_00CEF76D."),
    ("0048b410", "command_quit_game_0048b410", "src/presentation/NativeCommandLeafs.cpp", "compiled_only", "Pause-menu state transition only when current state is 1."),
    ("0048b430", "command_resume_play_0048b430", "src/presentation/NativeCommandLeafs.cpp", "compiled_only", "Pause-menu state transition only when current state is 1."),
    ("0048c130", "command_quit_to_windows_0048c130", "src/presentation/NativeCommandLeafs.cpp", "compiled_only", "USER32 PostQuitMessage import verified in PE import table."),
    ("0048c950", "command_language_menu_select_0048c950", "src/presentation/NativeCommandLeafs.cpp", "compiled_only", "MSVCR80 atoi import verified in PE import table."),
    ("005db380", "service_context_registry_name_005db380", "src/platform/ServiceContextRegistry.cpp", "passed", "Hash table, canonical string pool, and overflow limits tested; native startup still blocked."),
    ("005db440", "hp6::platform::heap_node_payload_005db440", "src/platform/ServiceHeapLeafs.cpp", "compiled_only", "Two-instruction heap leaf."),
    ("005db470", "hp6::platform::heap_relative_base_005db470", "src/platform/ServiceHeapLeafs.cpp", "compiled_only", "Heap back-offset leaf."),
    ("005db480", "hp6::platform::heap_control_at_30_005db480", "src/platform/ServiceHeapLeafs.cpp", "compiled_only", "Heap control address leaf."),
    ("005db490", "hp6::platform::heap_header_flag_005db490", "src/platform/ServiceHeapLeafs.cpp", "compiled_only", "Heap header flag leaf."),
    ("005db4d0", "service_heap_aligned_split_address_005db4d0", "src/platform/ServiceHeapAlignmentLeafs.cpp", "passed", "Absolute alignment split address checked against the x86 body."),
    ("005db640", "service_primary_heap_growth_005db640", "src/platform/ServicePrimaryHeapGrowth.cpp", "passed", "Callback path tested with explicit fake; callee 0x005DBBB0 still incomplete."),
    ("005db680", "service_context_name_005db680", "src/platform/ServiceContextName.cpp", "passed", "Prefix concatenation tested; original has unbounded copies."),
    ("005db710", "hp6::platform::ServiceHolderObject::allocate_context", "src/platform/ServiceHolderAllocateContext.cpp", "passed", "Wrapper tested with explicit fake; core allocator 0x005DDE30 absent."),
    ("005db7a0", "hp6::platform::ServiceHolderObject::allocate_named", "src/platform/ServiceHolderAllocateNamed.cpp", "passed", "Argument mapping tested with explicit fake; core 0x005DBC50 absent."),
    ("005db800", "hp6::platform::ServiceHolderSecondary::allocate_from_options", "src/platform/ServiceHolderAllocateOptions.cpp", "passed", "Option scan tested with explicit fake; core 0x005DBC50 absent."),
    ("005dba40", "service_holder_heap_initialize_005dba40", "src/platform/ServiceHolderHeapInitialize.cpp", "compiled_only", "Lazy heap setup compiled; full native allocation remains unlinked."),
    ("005dbbb0", "service_secondary_heap_expand_005dbbb0", "src/platform/ServiceSecondaryHeapExpand.cpp", "passed", "Region rounding, attach, and failure release tested with explicit fakes; callee 0x005DE540 still absent."),
    ("005dbc50", "service_heap_allocate_core_005dbc50", "src/platform/ServiceHeapAllocateNamedCore.cpp", "passed", "Named and aligned allocation paths tested with explicit fakes; inner allocators remain unresolved."),
    ("005dd2c0", "service_heap_free_callback_005dd2c0", "src/platform/ServiceHeapDebugCallbacks.cpp", "compiled_only", "OutputDebugStringA diagnostic callback."),
    ("005ddba0", "service_heap_insert_region_005ddba0", "src/platform/ServiceHeapInsertRegion.cpp", "passed", "Head-list insertion and usable-end update tested."),
    ("005ddb00", "service_heap_large_bin_005ddb00", "src/platform/ServiceHeapAlignmentLeafs.cpp", "passed", "Six large-block bin ranges tested at boundaries."),
    ("005ddc00", "service_heap_initialize_free_block_005ddc00", "src/platform/ServiceHeapSplitBlocks.cpp", "passed", "Standalone free-block tags and end cursor tested."),
    ("005ddc80", "service_heap_release_callback_005ddc80", "src/platform/ServiceHeapDebugCallbacks.cpp", "compiled_only", "OutputDebugStringA and int3 diagnostic callback."),
    ("005de540", "service_heap_release_region_005de540", "src/platform/ServiceHeapReleaseRegion.cpp", "passed", "Lock/free/unlock wrapper tested with explicit free-payload fake."),
    ("005de470", "service_heap_split_tail_005de470", "src/platform/ServiceHeapSplitBlocks.cpp", "passed", "Free-prefix split and small-remainder branch tested."),
    ("005de4d0", "service_heap_split_prefix_005de4d0", "src/platform/ServiceHeapSplitBlocks.cpp", "passed", "Unlink and split of free block tested."),
    ("005de700", "service_heap_allocate_aligned_locked_005de700", "src/platform/ServiceHeapAllocateAlignedLocked.cpp", "passed", "Lock/aligned-core/unlock path tested with explicit inner allocator fake."),
    ("005df3a0", "service_heap_reserve_region_005df3a0", "src/platform/ServiceHeapReserveRegion.cpp", "passed", "Reserve/commit and region-boundary metadata tested with real VirtualAlloc."),
    ("00ff9850", "service_heap_allocate_00ff9850", "src/platform/ServiceHeapAllocateRegion.cpp", "passed", "Provided-region and heap-reserved branches tested; complete executable still has unresolved symbols."),
    ("005dddb0", "service_heap_allocate_locked_005dddb0", "src/platform/ServiceHeapAllocationWrappers.cpp", "passed", "Lock sequence tested with fake; core allocator 0x005DDE30 absent."),
    ("005dede0", "service_heap_usable_end_offset_005dede0", "src/platform/ServiceHeapAllocationWrappers.cpp", "passed", "Metadata offset branches tested."),
]


def main() -> None:
    image, base, _, sections = read_pe(PE)
    if hashlib.sha256(image).hexdigest() != UNPROTECTED_SHA256:
        raise RuntimeError("Unprotected HP6 PE hash mismatch")
    state = json.loads(STATE.read_text(encoding="utf-8"))
    now = datetime.now(timezone.utc).isoformat()
    added = 0
    for address, symbol, source, build, limitation in FUNCTIONS:
        section, data = bytes_at(image, base, sections, int(address, 16), 1)
        if data is None or section not in (".text", ".1911"):
            raise RuntimeError(f"{address}: no executable byte in verified PE")
        source_text = (ROOT / source).read_text(encoding="utf-8")
        if symbol.split("::")[-1] not in source_text:
            raise RuntimeError(f"{address}: missing {symbol} in {source}")
        existing = state["functions"].get(address)
        if existing is not None:
            if existing.get("batch") != "native_verified_20261022":
                raise RuntimeError(f"{address}: existing independent review must not be overwritten")
            continue
        state["functions"][address] = {
            "batch": "native_verified_20261022",
            "build": build,
            "cpp_symbol": symbol,
            "equivalence": "unverified",
            "integration": "integrated",
            "integration_note": "Traced from SHA-verified unprotected HP6 x86 PE. " + limitation,
            "provenance": "unprotected PE disassembly; not a ReAgent candidate",
            "source": source,
            "updated_at": now,
        }
        added += 1
    STATE.write_text(json.dumps(state, indent=2, ensure_ascii=False) + "\n", encoding="utf-8")
    print(f"Recorded {added} newly reviewed native functions in {STATE}")


if __name__ == "__main__":
    main()
