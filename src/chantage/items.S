/* Enable intel syntax */
.intel_syntax noprefix

.global Item_ASM_LoadInventoryItemQuantityPtr_RCX
Item_ASM_LoadInventoryItemQuantityPtr_RCX:
    lea rcx, [gInventoryItemQuantity]
    ret

.global Item_ASM_LoadInventoryItemQuantityPtr_RDX
Item_ASM_LoadInventoryItemQuantityPtr_RDX:
    lea rdx, [gInventoryItemQuantity]
    ret
