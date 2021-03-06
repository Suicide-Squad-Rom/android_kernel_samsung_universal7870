/*
 * SCSI library functions depending on DMA
 */

#include <linux/blkdev.h>
#include <linux/device.h>
#include <linux/export.h>
#include <linux/kernel.h>

#include <scsi/scsi.h>
#include <scsi/scsi_cmnd.h>
#include <scsi/scsi_device.h>
#include <scsi/scsi_host.h>

static struct dma_attrs scsi_direct_attrs;

/**
 * scsi_dma_map - perform DMA mapping against command's sg lists
 * @cmd:	scsi command
 *
 * Returns the number of sg lists actually used, zero if the sg lists
 * is NULL, or -ENOMEM if the mapping failed.
 */
int scsi_dma_map(struct scsi_cmnd *cmd)
{
	int nseg = 0;
	struct dma_attrs *attrs = &scsi_direct_attrs;

	if (scsi_sg_count(cmd)) {
		struct device *dev = cmd->device->host->dma_dev;

		if (dma_get_attr(DMA_ATTR_SKIP_CPU_SYNC, attrs))
			attrs = (cmd->request->cmd_flags & REQ_KERNEL) ?
				&scsi_direct_attrs : NULL;
		nseg = dma_map_sg_attr(dev, scsi_sglist(cmd),scsi_sg_count(cmd), cmd->sc_data_direction, attrs);
		if (unlikely(!nseg))
			return -ENOMEM;
	}
	return nseg;
}
EXPORT_SYMBOL(scsi_dma_map);

/**
 * scsi_dma_unmap - unmap command's sg lists mapped by scsi_dma_map
 * @cmd:	scsi command
 */
void scsi_dma_unmap(struct scsi_cmnd *cmd)
{
	if (scsi_sg_count(cmd)) {
		struct device *dev = cmd->device->host->dma_dev;

		dma_unmap_sg(dev, scsi_sglist(cmd), scsi_sg_count(cmd),
			     cmd->sc_data_direction);
	}
}
EXPORT_SYMBOL(scsi_dma_unmap);

/**
 * scsi_dma_set_skip_cpu_sync - skip operations for cache coherency
 */
void scsi_dma_set_skip_cpu_sync(void)
{
	init_dma_attrs(&scsi_direct_attrs);
	dma_set_attr(DMA_ATTR_SKIP_CPU_SYNC, &scsi_direct_attrs);
}
EXPORT_SYMBOL(scsi_dma_set_skip_cpu_sync);
